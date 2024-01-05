
mkdir -p out
mkdir -p out/full

cd ..
cd ..

python tools/merger/join.py --template tools/merger/sfw_full.h.inl --path . --output tools/merger/out/full/sfw.h
python tools/merger/join.py --template tools/merger/sfw_full.cpp.inl --path . --output tools/merger/out/full/sfw.cpp
python tools/merger/join.py --template tools/merger/sfw_3rd_full.x.inl --path . --output tools/merger/out/full/sfw_3rd.h

