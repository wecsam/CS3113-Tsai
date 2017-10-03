#!/usr/bin/env python3
import xml.etree.ElementTree
BOILERPLATE = '''#include <unordered_map>
struct Sprite {
	const std::string filename;
	const size_t x;
	const size_t y;
	const size_t width;
	const size_t height;
	Sprite(
		const std::string& filename,
		size_t x,
		size_t y,
		size_t width,
		size_t height
	) : filename(filename), x(x), y(y), width(width), height(height) {}
};
'''

def make_constructors(texture_atlas_files, source_file="Sprite.h"):
    '''
    Takes data from multiple XML documents for sprite sheets and writes the
    data into a single C++ source file
    '''
    # Begin writing C++ source file.
    with open(source_file, "w") as source:
        source.write(BOILERPLATE)
        source.write("const std::unordered_map<std::string, Sprite> SPRITES = {\n")
        try:
            for texture_atlas_file in texture_atlas_files:
                # Parse the XML file. Make sure that the root is TextureAtlas.
                root = xml.etree.ElementTree.parse(texture_atlas_file).getroot()
                if root.tag == "TextureAtlas":
                    # Make sure that the root has attribute imagePath.
                    image_path = root.attrib.get("imagePath", None)
                    if image_path:
                            # For every SubTexture element, make a Sprite object in the C++ source.
                            for child in root:
                                if child.tag == "SubTexture" and all(a in child.attrib for a in ("name", "x", "y", "width", "height")):
                                    source.write('\t{{"{}", {{"{}", {}, {}, {}, {}}}}},\n'.format(
                                        child.attrib["name"],
                                        image_path,
                                        child.attrib["x"],
                                        child.attrib["y"],
                                        child.attrib["width"],
                                        child.attrib["height"]
                                    ))
                                else:
                                    print("Unknown tag:", child.tag)
                    else:
                        print("Image path is not in root attributes in", texture_atlas_file)
                else:
                    print("Root is not TextureAtlas in", texture_atlas_file)
        finally:
            source.write("};\n")

if __name__ == "__main__":
    make_constructors([r"C:\Users\espli\Documents\GitHub\CS3113\assets\graphics\UI pack (300 assets)\Space pack (80 assets)\Spritesheet\uipackSpace_sheet.xml"])