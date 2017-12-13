#pragma once
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>
class TileFile {
public:
	struct ParseError {
		ParseError(const char* message, const std::string& line = "");
		const char* message;
		const std::string line;
	};
	struct Entity {
		Entity(int Row, int Column, int DoorX, int DoorY, bool FacingLeft);
		int Row, Column, DoorX, DoorY;
		bool FacingLeft;
	};
	typedef std::string LayerName;
	typedef std::unordered_map<LayerName, int**> Layers;
	typedef std::string EntityType;
	typedef std::unordered_map<EntityType, std::vector<Entity>> Entities;
	TileFile();
	TileFile(std::istream& tileFile);
	TileFile(const TileFile& rhs);
	TileFile(TileFile&& rhs);
	TileFile& operator=(const TileFile& rhs);
	~TileFile();
	unsigned int GetMapWidth() const;
	unsigned int GetMapHeight() const;
	const Layers& GetLayers() const;
	const Entities& GetEntities() const;
private:
	// Classes for parsing the file
	class TileFileSection {
	public:
		virtual void SectionStart(TileFile*) = 0;
		virtual void Data(TileFile*, const std::string& key, const std::string& value) = 0;
		virtual void SectionEnd(TileFile*) = 0;
	};
	class TileFileHeader : public TileFileSection {
		void SectionStart(TileFile*) override;
		void Data(TileFile*, const std::string& key, const std::string& value) override;
		void SectionEnd(TileFile*) override;
	};
	class TileFileTilesets : public TileFileSection {
		void SectionStart(TileFile*) override;
		void Data(TileFile*, const std::string& key, const std::string& value) override;
		void SectionEnd(TileFile*) override;
	};
	class TileFileLayer : public TileFileSection {
		void SectionStart(TileFile*) override;
		void Data(TileFile*, const std::string& key, const std::string& value) override;
		void SectionEnd(TileFile*) override;
		int** map = nullptr;
	};
	class TileFileEntities : public TileFileSection {
		void SectionStart(TileFile*) override;
		void Data(TileFile*, const std::string& key, const std::string& value) override;
		void SectionEnd(TileFile*) override;
		std::string type;
		std::string location;
		int doorX, doorY;
		bool facingLeft;
	};
	// Data about the map
	unsigned int mapWidth = 0;
	unsigned int mapHeight = 0;
	Layers layers;
	Entities entities;
	// Housekeeping functions
	void CopyLayers(const Layers&);
	void DestroyLayers();
};
