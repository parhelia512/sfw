
//--STRIP
#include "dir_access.h"

#include <cstdio>
//--STRIP

/*************************************************************************/
/*  dir_access.cpp                                                       */
/*  From https://github.com/Relintai/pandemonium_engine (MIT)            */
/*************************************************************************/

//--STRIP
#include "dir_access.h"

#include "core/list.h"

#include "core/file_access.h"
#include "core/memory.h"
#include "core/local_vector.h"
//--STRIP

#if defined(_WIN64) || defined(_WIN32)
#else

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/statvfs.h>

#ifdef HAVE_MNTENT
#include <mntent.h>
#endif

#endif

#if defined(_WIN64) || defined(_WIN32)
#else

Error DirAccess::list_dir_begin(bool skip_specials) {
	list_dir_end(); //close any previous dir opening!

	_skip_specials = skip_specials;

	//char real_current_dir_name[2048]; //is this enough?!
	//getcwd(real_current_dir_name,2048);
	//chdir(current_path.utf8().get_data());
	dir_stream = opendir(current_dir.utf8().get_data());
	//chdir(real_current_dir_name);
	if (!dir_stream) {
		return ERR_CANT_OPEN; //error!
	}

	return OK;
}

bool DirAccess::file_exists(String p_file) {
	GLOBAL_LOCK_FUNCTION

	if (p_file.is_rel_path()) {
		p_file = current_dir.plus_file(p_file);
	}

	struct stat flags;
	bool success = (stat(p_file.utf8().get_data(), &flags) == 0);

	if (success && S_ISDIR(flags.st_mode)) {
		success = false;
	}

	return success;
}

bool DirAccess::dir_exists(String p_dir) {
	GLOBAL_LOCK_FUNCTION

	if (p_dir.is_rel_path()) {
		p_dir = get_current_dir().plus_file(p_dir);
	}

	struct stat flags;
	bool success = (stat(p_dir.utf8().get_data(), &flags) == 0);

	return (success && S_ISDIR(flags.st_mode));
}

uint64_t DirAccess::get_modified_time(String p_file) {
	if (p_file.is_rel_path()) {
		p_file = current_dir.plus_file(p_file);
	}

	struct stat flags;
	bool success = (stat(p_file.utf8().get_data(), &flags) == 0);

	if (success) {
		return flags.st_mtime;
	} else {
		ERR_FAIL_V(0);
	};
	return 0;
};

String DirAccess::get_next() {
	if (!dir_stream) {
		return "";
	}

	dirent *entry = readdir(dir_stream);

	if (entry == nullptr) {
		list_dir_end();
		return "";
	}

	String fname = fix_unicode_name(entry->d_name);

	// Look at d_type to determine if the entry is a directory, unless
	// its type is unknown (the file system does not support it) or if
	// the type is a link, in that case we want to resolve the link to
	// known if it points to a directory. stat() will resolve the link
	// for us.
	if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
		String f = current_dir.plus_file(fname);

		struct stat flags;
		if (stat(f.utf8().get_data(), &flags) == 0) {
			_cisdir = S_ISDIR(flags.st_mode);
		} else {
			_cisdir = false;
		}
	} else {
		_cisdir = (entry->d_type == DT_DIR);
	}

	_cishidden = is_hidden(fname);

	_cisspecial = is_special(fname);

	if (_skip_specials && _cisspecial) {
		// Should only happen 2 times max
		return get_next();
	}

	return fname;
}

bool DirAccess::current_is_dir() const {
	return _cisdir;
}

bool DirAccess::current_is_file() const {
	return !_cisdir;
}

bool DirAccess::current_is_special_dir() const {
	return _cisspecial;
}

bool DirAccess::current_is_hidden() const {
	return _cishidden;
}

void DirAccess::list_dir_end() {
	if (dir_stream) {
		closedir(dir_stream);
	}
	dir_stream = nullptr;
	_cisdir = false;
}

#if defined(HAVE_MNTENT) && defined(X11_ENABLED)
static bool _filter_drive(struct mntent *mnt) {
	// Ignore devices that don't point to /dev
	if (strncmp(mnt->mnt_fsname, "/dev", 4) != 0) {
		return false;
	}

	// Accept devices mounted at common locations
	if (strncmp(mnt->mnt_dir, "/media", 6) == 0 ||
			strncmp(mnt->mnt_dir, "/mnt", 4) == 0 ||
			strncmp(mnt->mnt_dir, "/home", 5) == 0 ||
			strncmp(mnt->mnt_dir, "/run/media", 10) == 0) {
		return true;
	}

	// Ignore everything else
	return false;
}
#endif

static void _get_drives(List<String> *list) {
	list->push_back("/");

#if defined(HAVE_MNTENT) && defined(X11_ENABLED)
	// Check /etc/mtab for the list of mounted partitions
	FILE *mtab = setmntent("/etc/mtab", "r");
	if (mtab) {
		struct mntent mnt;
		char strings[4096];

		while (getmntent_r(mtab, &mnt, strings, sizeof(strings))) {
			if (mnt.mnt_dir != nullptr && _filter_drive(&mnt)) {
				// Avoid duplicates
				if (!list->find(mnt.mnt_dir)) {
					list->push_back(mnt.mnt_dir);
				}
			}
		}

		endmntent(mtab);
	}
#endif

	// Add $HOME
	const char *home = getenv("HOME");
	if (home) {
		// Only add if it's not a duplicate
		if (!list->find(home)) {
			list->push_back(home);
		}

		// Check $HOME/.config/gtk-3.0/bookmarks
		char path[1024];
		snprintf(path, 1024, "%s/.config/gtk-3.0/bookmarks", home);
		FILE *fd = fopen(path, "r");
		if (fd) {
			char string[1024];
			while (fgets(string, 1024, fd)) {
				// Parse only file:// links
				if (strncmp(string, "file://", 7) == 0) {
					// Strip any unwanted edges on the strings and push_back if it's not a duplicate
					String fpath = String(string + 7).strip_edges().split_spaces()[0].percent_decode();
					if (!list->find(fpath)) {
						list->push_back(fpath);
					}
				}
			}

			fclose(fd);
		}
	}

	list->sort();
}

int DirAccess::get_drive_count() {
	List<String> list;
	_get_drives(&list);

	return list.size();
}

String DirAccess::get_drive(int p_drive) {
	List<String> list;
	_get_drives(&list);

	ERR_FAIL_INDEX_V(p_drive, list.size(), "");

	return list[p_drive];
}

int DirAccess::get_current_drive() {
	int drive = 0;
	int max_length = -1;
	const String path = get_current_dir().to_lower();
	for (int i = 0; i < get_drive_count(); i++) {
		const String d = get_drive(i).to_lower();
		if (max_length < d.length() && path.begins_with(d)) {
			max_length = d.length();
			drive = i;
		}
	}
	return drive;
}

bool DirAccess::drives_are_shortcuts() {
	return true;
}

Error DirAccess::make_dir(String p_dir) {
	GLOBAL_LOCK_FUNCTION

	if (p_dir.is_rel_path()) {
		p_dir = get_current_dir().plus_file(p_dir);
	}

	bool success = (mkdir(p_dir.utf8().get_data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
	int err = errno;

	if (success) {
		return OK;
	};

	if (err == EEXIST) {
		return ERR_ALREADY_EXISTS;
	};

	return ERR_CANT_CREATE;
}

Error DirAccess::change_dir(String p_dir) {
	GLOBAL_LOCK_FUNCTION

	// prev_dir is the directory we are changing out of
	String prev_dir;
	char real_current_dir_name[2048];
	ERR_FAIL_COND_V(getcwd(real_current_dir_name, 2048) == nullptr, ERR_BUG);
	if (prev_dir.parse_utf8(real_current_dir_name)) {
		prev_dir = real_current_dir_name; //no utf8, maybe latin?
	}

	// try_dir is the directory we are trying to change into
	String try_dir = "";
	if (p_dir.is_rel_path()) {
		String next_dir = current_dir.plus_file(p_dir);
		next_dir = next_dir.simplify_path();
		try_dir = next_dir;
	} else {
		try_dir = p_dir;
	}

	bool worked = (chdir(try_dir.utf8().get_data()) == 0); // we can only give this utf8
	if (!worked) {
		return ERR_INVALID_PARAMETER;
	}

	String base;
	if (base != String() && !try_dir.begins_with(base)) {
		ERR_FAIL_COND_V(getcwd(real_current_dir_name, 2048) == nullptr, ERR_BUG);
		String new_dir;
		new_dir.parse_utf8(real_current_dir_name);

		if (!new_dir.begins_with(base)) {
			try_dir = current_dir; //revert
		}
	}

	// the directory exists, so set current_dir to try_dir
	current_dir = try_dir;
	ERR_FAIL_COND_V(chdir(prev_dir.utf8().get_data()) != 0, ERR_BUG);
	return OK;
}

String DirAccess::get_current_dir() {
	String base;
	if (base != "") {
		String bd = current_dir.replace_first(base, "");
		if (bd.begins_with("/")) {
			return bd.substr(1, bd.length());
		} else {
			return bd;
		}
	}
	return current_dir;
}

Error DirAccess::rename(String p_path, String p_new_path) {
	if (p_path.is_rel_path()) {
		p_path = get_current_dir().plus_file(p_path);
	}

	if (p_new_path.is_rel_path()) {
		p_new_path = get_current_dir().plus_file(p_new_path);
	}

	return ::rename(p_path.utf8().get_data(), p_new_path.utf8().get_data()) == 0 ? OK : FAILED;
}

Error DirAccess::remove(String p_path) {
	if (p_path.is_rel_path()) {
		p_path = get_current_dir().plus_file(p_path);
	}

	struct stat flags;
	if ((stat(p_path.utf8().get_data(), &flags) != 0)) {
		return FAILED;
	}

	if (S_ISDIR(flags.st_mode)) {
		return ::rmdir(p_path.utf8().get_data()) == 0 ? OK : FAILED;
	} else {
		return ::unlink(p_path.utf8().get_data()) == 0 ? OK : FAILED;
	}
}

bool DirAccess::is_link(String p_file) {
	if (p_file.is_rel_path()) {
		p_file = get_current_dir().plus_file(p_file);
	}

	struct stat flags;
	if ((lstat(p_file.utf8().get_data(), &flags) != 0))
		return FAILED;

	return S_ISLNK(flags.st_mode);
}

String DirAccess::read_link(String p_file) {
	if (p_file.is_rel_path()) {
		p_file = get_current_dir().plus_file(p_file);
	}

	char buf[256];
	memset(buf, 0, 256);
	ssize_t len = readlink(p_file.utf8().get_data(), buf, sizeof(buf));
	String link;
	if (len > 0) {
		link.parse_utf8(buf, len);
	}
	return link;
}

Error DirAccess::create_link(String p_source, String p_target) {
	if (p_target.is_rel_path())
		p_target = get_current_dir().plus_file(p_target);

	if (symlink(p_source.utf8().get_data(), p_target.utf8().get_data()) == 0) {
		return OK;
	} else {
		return FAILED;
	}
}

uint64_t DirAccess::get_space_left() {
#ifndef NO_STATVFS
	struct statvfs vfs;
	if (statvfs(current_dir.utf8().get_data(), &vfs) != 0) {
		return 0;
	};

	return (uint64_t)vfs.f_bavail * (uint64_t)vfs.f_frsize;
#else
	// FIXME: Implement this.
	return 0;
#endif
};

String DirAccess::get_filesystem_type() const {
	return ""; //TODO this should be implemented
}

bool DirAccess::is_hidden(const String &p_name) {
	return p_name != "." && p_name != ".." && p_name.begins_with(".");
}

DirAccess::DirAccess() {
	dir_stream = NULL;
	_cisdir = false;

	next_is_dir = false;
	_skip_specials = false;

	_cishidden = false;
	_cisspecial = false;

	/* determine drive count */

	// set current directory to an absolute path of the current directory
	char real_current_dir_name[2048];
	ERR_FAIL_COND(getcwd(real_current_dir_name, 2048) == nullptr);
	if (current_dir.parse_utf8(real_current_dir_name)) {
		current_dir = real_current_dir_name;
	}

	change_dir(current_dir);
}

DirAccess::~DirAccess() {
	list_dir_end();
}

#endif

/*
int DirAccess::get_current_drive() {
	String path = get_current_dir().to_lower();
	for (int i = 0; i < get_drive_count(); i++) {
		String d = get_drive(i).to_lower();
		if (path.begins_with(d)) {
			return i;
		}
	}

	return 0;
}

bool DirAccess::drives_are_shortcuts() {
	return false;
}
*/

String DirAccess::get_current_dir_without_drive() {
	return get_current_dir();
}

static Error _erase_recursive(DirAccess *da) {
	List<String> dirs;
	List<String> files;

	da->list_dir_begin();
	String n = da->get_next();
	while (n != String()) {
		if (n != "." && n != "..") {
			if (da->current_is_dir()) {
				dirs.push_back(n);
			} else {
				files.push_back(n);
			}
		}

		n = da->get_next();
	}

	da->list_dir_end();

	for (List<String>::Element *E = dirs.front(); E; E = E->next()) {
		Error err = da->change_dir(E->get());
		if (err == OK) {
			err = _erase_recursive(da);
			if (err) {
				da->change_dir("..");
				return err;
			}
			err = da->change_dir("..");
			if (err) {
				return err;
			}
			err = da->remove(da->get_current_dir().plus_file(E->get()));
			if (err) {
				return err;
			}
		} else {
			return err;
		}
	}

	for (List<String>::Element *E = files.front(); E; E = E->next()) {
		Error err = da->remove(da->get_current_dir().plus_file(E->get()));
		if (err) {
			return err;
		}
	}

	return OK;
}

Error DirAccess::erase_contents_recursive() {
	return _erase_recursive(this);
}

Error DirAccess::make_dir_recursive(String p_dir) {
	if (p_dir.length() < 1) {
		return OK;
	};

	String full_dir;

	if (p_dir.is_rel_path()) {
		//append current
		full_dir = get_current_dir().plus_file(p_dir);

	} else {
		full_dir = p_dir;
	}

	full_dir = full_dir.replace("\\", "/");

	String base;

	if (full_dir.begins_with("res://")) {
		base = "res://";
	} else if (full_dir.begins_with("user://")) {
		base = "user://";
	} else if (full_dir.is_network_share_path()) {
		int pos = full_dir.find("/", 2);
		ERR_FAIL_COND_V(pos < 0, ERR_INVALID_PARAMETER);
		pos = full_dir.find("/", pos + 1);
		ERR_FAIL_COND_V(pos < 0, ERR_INVALID_PARAMETER);
		base = full_dir.substr(0, pos + 1);
	} else if (full_dir.begins_with("/")) {
		base = "/";
	} else if (full_dir.find(":/") != -1) {
		base = full_dir.substr(0, full_dir.find(":/") + 2);
	} else {
		ERR_FAIL_V(ERR_INVALID_PARAMETER);
	}

	full_dir = full_dir.replace_first(base, "").simplify_path();

	Vector<String> subdirs = full_dir.split("/");

	String curpath = base;
	for (int i = 0; i < subdirs.size(); i++) {
		curpath = curpath.plus_file(subdirs[i]);
		Error err = make_dir(curpath);
		if (err != OK && err != ERR_ALREADY_EXISTS) {
			ERR_FAIL_V_MSG(err, "Could not create directory: " + curpath);
		}
	}

	return OK;
}

DirAccess *DirAccess::create_for_path(const String &p_path) {
	DirAccess *d = memnew(DirAccess());
	d->open(p_path);
	return d;
}
DirAccess *DirAccess::create() {
	return memnew(DirAccess());
}

Error DirAccess::open(const String &p_path) {
	return change_dir(p_path);
}

String DirAccess::get_full_path(const String &p_path) {
	DirAccess d;

	d.change_dir(p_path);
	String full = d.get_current_dir();

	return full;
}

Error DirAccess::copy(String p_from, String p_to, int p_chmod_flags) {
	//printf("copy %s -> %s\n",p_from.ascii().get_data(),p_to.ascii().get_data());
	Error err;
	FileAccess *fsrc = FileAccess::open(p_from, FileAccess::READ, &err);

	if (err) {
		ERR_PRINT("Failed to open " + p_from);
		return err;
	}

	FileAccess *fdst = FileAccess::open(p_to, FileAccess::WRITE, &err);
	if (err) {
		fsrc->close();
		memdelete(fsrc);
		ERR_PRINT("Failed to open " + p_to);
		return err;
	}

	const size_t copy_buffer_limit = 65536; // 64 KB

	fsrc->seek_end(0);
	uint64_t size = fsrc->get_position();
	fsrc->seek(0);
	err = OK;
	size_t buffer_size = MIN(size * sizeof(uint8_t), copy_buffer_limit);
	LocalVector<uint8_t> buffer;
	buffer.resize(buffer_size);
	while (size > 0) {
		if (fsrc->get_error() != OK) {
			err = fsrc->get_error();
			break;
		}
		if (fdst->get_error() != OK) {
			err = fdst->get_error();
			break;
		}

		int bytes_read = fsrc->get_buffer(buffer.ptr(), buffer_size);
		if (bytes_read <= 0) {
			err = FAILED;
			break;
		}
		fdst->store_buffer(buffer.ptr(), bytes_read);

		size -= bytes_read;
	}

	if (err == OK && p_chmod_flags != -1) {
		fdst->close();
		err = FileAccess::set_unix_permissions(p_to, p_chmod_flags);
		// If running on a platform with no chmod support (i.e., Windows), don't fail
		if (err == ERR_UNAVAILABLE) {
			err = OK;
		}
	}

	memdelete(fsrc);
	memdelete(fdst);

	return err;
}

// Changes dir for the current scope, returning back to the original dir
// when scope exits
class DirChanger {
	DirAccess *da;
	String original_dir;

public:
	DirChanger(DirAccess *p_da, String p_dir) :
			da(p_da),
			original_dir(p_da->get_current_dir()) {
		p_da->change_dir(p_dir);
	}

	~DirChanger() {
		da->change_dir(original_dir);
	}
};

Error DirAccess::_copy_dir(DirAccess *p_target_da, String p_to, int p_chmod_flags, bool p_copy_links) {
	List<String> dirs;

	String curdir = get_current_dir();
	list_dir_begin();
	String n = get_next();
	while (n != String()) {
		if (n != "." && n != "..") {
			if (p_copy_links && is_link(get_current_dir().plus_file(n))) {
				create_link(read_link(get_current_dir().plus_file(n)), p_to + n);
			} else if (current_is_dir()) {
				dirs.push_back(n);
			} else {
				const String &rel_path = n;
				if (!n.is_rel_path()) {
					list_dir_end();
					return ERR_BUG;
				}
				Error err = copy(get_current_dir().plus_file(n), p_to + rel_path, p_chmod_flags);
				if (err) {
					list_dir_end();
					return err;
				}
			}
		}

		n = get_next();
	}

	list_dir_end();

	for (List<String>::Element *E = dirs.front(); E; E = E->next()) {
		String rel_path = E->get();
		String target_dir = p_to + rel_path;
		if (!p_target_da->dir_exists(target_dir)) {
			Error err = p_target_da->make_dir(target_dir);
			ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot create directory '" + target_dir + "'.");
		}

		Error err = change_dir(E->get());
		ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot change current directory to '" + E->get() + "'.");

		err = _copy_dir(p_target_da, p_to + rel_path + "/", p_chmod_flags, p_copy_links);
		if (err) {
			change_dir("..");
			ERR_FAIL_V_MSG(err, "Failed to copy recursively.");
		}
		err = change_dir("..");
		ERR_FAIL_COND_V_MSG(err != OK, err, "Failed to go back.");
	}

	return OK;
}

Error DirAccess::copy_dir(String p_from, String p_to, int p_chmod_flags, bool p_copy_links) {
	ERR_FAIL_COND_V_MSG(!dir_exists(p_from), ERR_FILE_NOT_FOUND, "Source directory doesn't exist.");

	DirAccess *target_da = DirAccess::create_for_path(p_to);
	ERR_FAIL_COND_V_MSG(!target_da, ERR_CANT_CREATE, "Cannot create DirAccess for path '" + p_to + "'.");

	if (!target_da->dir_exists(p_to)) {
		Error err = target_da->make_dir_recursive(p_to);
		if (err) {
			memdelete(target_da);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot create directory '" + p_to + "'.");
	}

	if (!p_to.ends_with("/")) {
		p_to = p_to + "/";
	}

	DirChanger dir_changer(this, p_from);
	Error err = _copy_dir(target_da, p_to, p_chmod_flags, p_copy_links);
	memdelete(target_da);

	return err;
}

bool DirAccess::exists(String p_dir) {
	DirAccess *da = DirAccess::create_for_path(p_dir);
	bool valid = da->change_dir(p_dir) == OK;
	memdelete(da);
	return valid;
}

String DirAccess::get_filesystem_abspath_for(String p_path) {
	DirAccess d;

	d.change_dir(p_path);
	String full = d.get_current_dir();

	return full;
}

bool DirAccess::is_special(const String &p_path) {
	if (p_path.size() > 2) {
		return false;
	}

	return p_path == "." || p_path == "..";
}
