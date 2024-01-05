/*************************************************************************/
/*  input_map.cpp                                                        */
/*  From https://github.com/Relintai/pandemonium_engine (MIT)            */
/*************************************************************************/

//--STRIP
#include "input_map.h"

#include "render_core/input.h"
#include "render_core/keyboard.h"
//--STRIP

InputMap *InputMap::singleton = nullptr;

int InputMap::ALL_DEVICES = -1;

/**
 * Returns an nonexistent action error message with a suggestion of the closest
 * matching action name (if possible).
 */
String InputMap::suggest_actions(const StringName &p_action) const {
	List<StringName> actions = get_actions();
	StringName closest_action;
	float closest_similarity = 0.0;

	// Find the most action with the most similar name.
	for (List<StringName>::Element *E = actions.front(); E; E = E->next()) {
		const float similarity = String(E->get()).similarity(p_action);

		if (similarity > closest_similarity) {
			closest_action = E->get();
			closest_similarity = similarity;
		}
	}

	String error_message = vformat("The InputMap action \"%s\" doesn't exist.", p_action);

	if (closest_similarity >= 0.4) {
		// Only include a suggestion in the error message if it's similar enough.
		error_message += vformat(" Did you mean \"%s\"?", closest_action);
	}
	return error_message;
}

void InputMap::add_action(const StringName &p_action, float p_deadzone) {
	ERR_FAIL_COND_MSG(input_map.has(p_action), "InputMap already has action \"" + String(p_action) + "\".");
	input_map[p_action] = Action();
	static int last_id = 1;
	input_map[p_action].id = last_id;
	input_map[p_action].deadzone = p_deadzone;
	last_id++;
}

void InputMap::erase_action(const StringName &p_action) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	input_map.erase(p_action);
}

Array InputMap::_get_actions() {
	Array ret;
	List<StringName> actions = get_actions();
	if (actions.empty()) {
		return ret;
	}

	for (const List<StringName>::Element *E = actions.front(); E; E = E->next()) {
		ret.push_back(E->get());
	}

	return ret;
}

List<StringName> InputMap::get_actions() const {
	List<StringName> actions = List<StringName>();
	if (input_map.empty()) {
		return actions;
	}

	for (RBMap<StringName, Action>::Element *E = input_map.front(); E; E = E->next()) {
		actions.push_back(E->key());
	}

	return actions;
}

List<Ref<InputEvent>>::Element *InputMap::_find_event(Action &p_action, const Ref<InputEvent> &p_event, bool p_exact_match, bool *p_pressed, float *p_strength, float *p_raw_strength) const {
	ERR_FAIL_COND_V(!p_event.is_valid(), nullptr);

	for (List<Ref<InputEvent>>::Element *E = p_action.inputs.front(); E; E = E->next()) {
		const Ref<InputEvent> e = E->get();

		//if (e.type != Ref<InputEvent>::KEY && e.device != p_event.device) -- unsure about the KEY comparison, why is this here?
		//	continue;

		int device = e->get_device();
		if (device == ALL_DEVICES || device == p_event->get_device()) {
			if (e->action_match(p_event, p_exact_match, p_pressed, p_strength, p_raw_strength, p_action.deadzone)) {
				return E;
			}
		}
	}

	return nullptr;
}

bool InputMap::has_action(const StringName &p_action) const {
	return input_map.has(p_action);
}

float InputMap::action_get_deadzone(const StringName &p_action) {
	ERR_FAIL_COND_V_MSG(!input_map.has(p_action), 0.0f, suggest_actions(p_action));

	return input_map[p_action].deadzone;
}

void InputMap::action_set_deadzone(const StringName &p_action, float p_deadzone) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	input_map[p_action].deadzone = p_deadzone;
}

void InputMap::action_add_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_MSG(p_event.is_null(), "It's not a reference to a valid InputEvent object.");
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	if (_find_event(input_map[p_action], p_event, true)) {
		return; // Already added.
	}

	input_map[p_action].inputs.push_back(p_event);
}

bool InputMap::action_has_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_V_MSG(!input_map.has(p_action), false, suggest_actions(p_action));

	return (_find_event(input_map[p_action], p_event, true) != nullptr);
}

void InputMap::action_erase_event(const StringName &p_action, const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	List<Ref<InputEvent>>::Element *E = _find_event(input_map[p_action], p_event, true);
	if (E) {
		input_map[p_action].inputs.erase(E);
		if (Input::get_singleton()->is_action_pressed(p_action)) {
			Input::get_singleton()->action_release(p_action);
		}
	}
}

void InputMap::action_erase_events(const StringName &p_action) {
	ERR_FAIL_COND_MSG(!input_map.has(p_action), suggest_actions(p_action));

	input_map[p_action].inputs.clear();
}

Array InputMap::_get_action_list(const StringName &p_action) {
	Array ret;
	const List<Ref<InputEvent>> *al = get_action_list(p_action);
	if (al) {
		for (const List<Ref<InputEvent>>::Element *E = al->front(); E; E = E->next()) {
			ret.push_back(E->get());
		}
	}

	return ret;
}

const List<Ref<InputEvent>> *InputMap::get_action_list(const StringName &p_action) {
	const RBMap<StringName, Action>::Element *E = input_map.find(p_action);
	if (!E) {
		return nullptr;
	}

	return &E->get().inputs;
}

bool InputMap::event_is_action(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match) const {
	return event_get_action_status(p_event, p_action, p_exact_match);
}

bool InputMap::event_get_action_status(const Ref<InputEvent> &p_event, const StringName &p_action, bool p_exact_match, bool *p_pressed, float *p_strength, float *p_raw_strength) const {
	RBMap<StringName, Action>::Element *E = input_map.find(p_action);
	ERR_FAIL_COND_V_MSG(!E, false, suggest_actions(p_action));

	Ref<InputEventAction> input_event_action = p_event;
	if (input_event_action.is_valid()) {
		bool pressed = input_event_action->is_pressed();
		if (p_pressed != nullptr) {
			*p_pressed = pressed;
		}
		if (p_strength != nullptr) {
			*p_strength = pressed ? input_event_action->get_strength() : 0.0f;
		}
		return input_event_action->get_action() == p_action;
	}

	bool pressed;
	float strength;
	float raw_strength;
	List<Ref<InputEvent>>::Element *event = _find_event(E->get(), p_event, p_exact_match, &pressed, &strength, &raw_strength);
	if (event != nullptr) {
		if (p_pressed != nullptr) {
			*p_pressed = pressed;
		}
		if (p_strength != nullptr) {
			*p_strength = strength;
		}
		if (p_raw_strength != nullptr) {
			*p_raw_strength = raw_strength;
		}
		return true;
	} else {
		return false;
	}
}

const RBMap<StringName, InputMap::Action> &InputMap::get_action_map() const {
	return input_map;
}

void InputMap::load_default() {
	Ref<InputEventKey> key;

	add_action("ui_accept");
	key.instance();
	key->set_scancode(KEY_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_scancode(KEY_KP_ENTER);
	action_add_event("ui_accept", key);

	key.instance();
	key->set_scancode(KEY_SPACE);
	action_add_event("ui_accept", key);

	add_action("ui_select");
	key.instance();
	key->set_scancode(KEY_SPACE);
	action_add_event("ui_select", key);

	add_action("ui_cancel");
	key.instance();
	key->set_scancode(KEY_ESCAPE);
	action_add_event("ui_cancel", key);

	add_action("ui_focus_next");
	key.instance();
	key->set_scancode(KEY_TAB);
	action_add_event("ui_focus_next", key);

	add_action("ui_focus_prev");
	key.instance();
	key->set_scancode(KEY_TAB);
	key->set_shift(true);
	action_add_event("ui_focus_prev", key);

	add_action("ui_left");
	key.instance();
	key->set_scancode(KEY_LEFT);
	action_add_event("ui_left", key);

	add_action("ui_right");
	key.instance();
	key->set_scancode(KEY_RIGHT);
	action_add_event("ui_right", key);

	add_action("ui_up");
	key.instance();
	key->set_scancode(KEY_UP);
	action_add_event("ui_up", key);

	add_action("ui_down");
	key.instance();
	key->set_scancode(KEY_DOWN);
	action_add_event("ui_down", key);

	add_action("ui_page_up");
	key.instance();
	key->set_scancode(KEY_PAGEUP);
	action_add_event("ui_page_up", key);

	add_action("ui_page_down");
	key.instance();
	key->set_scancode(KEY_PAGEDOWN);
	action_add_event("ui_page_down", key);

	add_action("ui_home");
	key.instance();
	key->set_scancode(KEY_HOME);
	action_add_event("ui_home", key);

	add_action("ui_end");
	key.instance();
	key->set_scancode(KEY_END);
	action_add_event("ui_end", key);
}

InputMap::InputMap() {
	ERR_FAIL_COND_MSG(singleton, "Singleton in InputMap already exist.");
	singleton = this;
}
