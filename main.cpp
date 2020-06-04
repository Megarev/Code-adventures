#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "GraphicsRender.h"
#include "AssetManager.h"
#include "GraphicsUI.h"
#include <iterator>
#include <ctime>
#include <list>
#include <Windows.h>

const float pixelSize = 32.0f;

std::vector<std::string> ToWords(const std::string& string) {
	//String to words
	std::istringstream iss(string);
	std::vector<std::string> v;
	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		std::back_inserter(v));

	return v;
};

class TextWindow {
private:
	sf::RectangleShape box;			  //Text Window

	std::vector<std::string> strings; //Lines of the text window
	std::vector<sf::Color> colors;	  //Colors of each line
	std::ostringstream textString;    //Active string line
	int textIndex, showTextIndex;	  //textIndex -> index of active string, showTextIndex for rendering
	uint32_t nShowText;				  //nShowText for rendering
	sf::Text text;					  //Text for rendering to the window

	void Input(int character) {

		if (character < 0 || character == 0x1B) return;

		if (character == 0x08 || character == 0x2E) {
			if (textString.str().size() > 0) {
				std::string initString = textString.str();
				initString.pop_back();
				textString.str("");
				textString << std::move(initString);
			}
			else {
				if (strings.size() > 0 && textIndex > 0) {
					auto it = strings.begin();
					int value = 0;
					while (value != textIndex) {
						value++;
						it++;
					}
					strings.erase(it);
					textIndex--;
					textString.str("");
					textString << strings[textIndex];
					colors.pop_back();

					if (showTextIndex > 0) showTextIndex--;
				}
			}
		}
		else {
			if (textString.str().size() == 0 && character == 0x20) return;
			textString << static_cast<char>(character);
		}
	}

	void AddNewLine() {
		auto it = strings.begin();
		int value = 0;
		while (value != textIndex) {
			it++;
			value++;
		}
		strings.insert(it, textString.str());
		textString.str("");
		colors.push_back(sf::Color::White);
		textIndex++;

		if (textIndex > (int)nShowText) {
			showTextIndex++;
		}
	}

	void ChangeActiveString(int dir) {
		if (dir == 0) {
			//Direction Up

			textIndex--;
			if (textIndex < 0) textIndex = 0;

			if (textIndex < (int)nShowText && showTextIndex > 0) showTextIndex--;
			textString.str("");
			textString << strings[textIndex];
		}
		else {
			//Direction Down

			textIndex++;
			if (textIndex > (int)(strings.size() - 1)) textIndex = (int)(strings.size() - 1);

			if (textIndex > (int)nShowText && showTextIndex < (int)(strings.size() - 1)) showTextIndex++;
			textString.str("");
			textString << strings[textIndex];
		}
	}

	void Push(const std::string& str) {
		colors.push_back(sf::Color::White);
		strings.push_back(str);
	};

public:
	TextWindow() {}
	TextWindow(const sf::Vector2f& pos, const sf::Vector2f& size) {
		box.setSize(size);
		box.setPosition(pos);
		box.setFillColor(sf::Color(25, 25, 25));
		text.setCharacterSize(15);

		Push("");

		textIndex = 0;
		showTextIndex = 0;

		const sf::Vector2u windowSize = { 512, 512 };
		nShowText = (uint32_t)(windowSize.y / text.getCharacterSize() - 3);
	}

	void SetFont(const sf::Font& font) {
		text.setFont(font);
	}

	void ResetStrings() {
		textString.str("");
		strings.clear();
		strings.push_back("");
		colors.clear();
		colors.push_back(sf::Color::White);

		textIndex = 0;
		showTextIndex = 0;
	}

	void ManageEvent(sf::Event e) {
		switch (e.type) {
		case sf::Event::TextEntered:
			if (e.text.unicode < 128) {
				Input(e.text.unicode);
			}
			break;
		case sf::Event::Resized:
			nShowText = (uint32_t)(e.size.height / text.getCharacterSize() - 3);
			break;
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::Up:
				ChangeActiveString(0);
				break;
			case sf::Keyboard::Down:
				ChangeActiveString(1);
				break;
			case sf::Keyboard::Return:
				AddNewLine();
				break;
			}
			break;
		}

		strings[textIndex] = textString.str();
	}

	void Logic() {

		int value = 0;
		for (int i = 0; i < (int)strings.size(); i++) {

			//Value variable is to make sure that loop instructions are not re-read
			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) stringIndex = ((int)strings.size() - 1);

			auto stringText = ToWords(strings[stringIndex]);
			if (stringText.size() == 0) continue;

			colors[stringIndex] = sf::Color::White;

			if (stringText.size() > 1) {
				if (stringText[0] == "turn") {
					colors[stringIndex] = sf::Color::Yellow;
					if (i == textIndex) {
						std::string activeString = textString.str();

						while (activeString.size() > (uint32_t)(i == 0 ? 7 : 8)) {
							activeString.pop_back();
						}

						textString.str("");
						textString << std::move(activeString);
					}
				}
				else if (stringText[0] == "move") {
					colors[stringIndex] = sf::Color::Cyan;
					if (i == textIndex) {
						std::string activeString = textString.str();

						while (activeString.size() > (uint32_t)(i == 0 ? 7 : 8)) {
							activeString.pop_back();
						}

						textString.str("");
						textString << std::move(activeString);
					}
				}
				if (stringText[0] == "openloop" || stringText[0] == "repeat") {
					for (int j = stringIndex; j < (int)strings.size(); j++) {

						auto loopText = ToWords(strings[j]);
						if (loopText.size() == 0) continue;

						
						colors[j] = sf::Color(255, 255, 255);
						if (loopText[0] == "openloop" || loopText[0] == "repeat") {
							colors[j] = sf::Color(255, 100, 0);
						}

						if (loopText[0] == "move") {
							colors[j] = sf::Color(0, 200, 100);
						}

						if (loopText[0] == "turn") {
							colors[j] = sf::Color(200, 100, 0);
						}

						if (loopText[0] == "if") colors[j] = sf::Color(100, 255, 255);

						if (loopText[0] == "else" || loopText[0] == "closeif" || loopText[0] == "endif") colors[j] = sf::Color(100, 255, 255);

						if (loopText[0] == "closeloop" || loopText[0] == "end") { //Openloop close
							colors[j] = sf::Color(255, 100, 0);
							break;
						}
						value++;
					}
				}
				if (stringText[0] == "if" || stringText[0] == "else" || stringText[0] == "endif" || stringText[0] == "closeif") {
					colors[i] = sf::Color(100, 255, 255);
				}
			}
		}
	}

	void Render(sf::RenderWindow& window) {
		window.draw(box);

		float pos = 2.0f;

		for (int i = showTextIndex; i < (int)strings.size(); i++) {

			auto stringText = ToWords(strings[i]);

			text.setPosition(window.getSize().x - 152.0f, pos * (float)text.getCharacterSize());
			text.setFillColor(sf::Color(255, 255, 255, 100));

			int index = 0;
			const std::string& strView = strings[i];
			if (strView[0] == '\r') index = 1;

			if ((int)strView.size() > index) {
				if (strView[index] == 'm') {
					text.setString(sf::String("> move fd"));
					window.draw(text);
				}
				if (strView[index] == 'r' && strView.size() < 8) {
					text.setString(sf::String("> repeat n\n> [statements...]\n> end"));
					window.draw(text);
				}
				if (strView[index] == 't' && strView.size() < 5) {
					text.setString(sf::String("> turn [dir]"));
					window.draw(text);
				}
				if (strView[index] == 'i') {
					if (strView.size() < 12) {
						text.setString(sf::String("> if fd empty\n> [action1]\n> else\n> [action2]\n> endif"));
						window.draw(text);
					}
				}
			}

			text.setFillColor(colors[i]);
			text.setString("> " + strings[i] + (i == textIndex ? "_" : ""));
			window.draw(text);

			if (stringText.size() > 1) {
				if (stringText[0] == "move" || stringText[0] == "openloop" || stringText[0] == "repeat" || stringText[0] == "turn") {
					if (stringText[0] == "move") text.setFillColor(sf::Color::Green);
					else if (stringText[0] == "openloop" || stringText[0] == "repeat") text.setFillColor(sf::Color::Magenta);
					else if (stringText[0] == "turn") text.setFillColor(sf::Color(200, 100, 0));
					text.setString(stringText[1]);

					float positionX = (float)window.getSize().x - stringText[0].size() * text.getCharacterSize();
					if (stringText[0] == "move" || stringText[0] == "turn") { positionX -= 30.0f; }
					else if (stringText[0] == "openloop") { positionX += 67.0f; }
					else if (stringText[0] == "repeat") { positionX += 18.0f; }

					text.setPosition(positionX, pos * (float)text.getCharacterSize());
					window.draw(text);
				}
			}

			pos++;
		}
	}

	void SetPosition(float x, float y) {
		box.setPosition(x, y);
	}

	void SetSize(float x, float y) {
		box.setSize({ x, y });
	}

	std::vector<sf::Color>& Colors() { return colors; }
	inline std::vector<std::string> GetStrings() const { return strings; }
};

typedef std::vector<std::vector<std::string>> Texts;
class TextManager {
private:
	Texts texts;
	int index;
public:
	TextManager() {
		index = 0;
	}

	void LoadTexts(const std::string& filepath) {
		std::ifstream reader(filepath);

		if (reader.is_open()) {
			std::vector<std::string> initTexts;
			std::string str;

			while (!reader.eof()) {

				std::vector<std::string> initTexts;

				while (true) {
					std::string str;
					std::getline(reader, str);
					if (str == ">") break;

					initTexts.push_back(str);
				}

				texts.push_back(initTexts);
			}

			reader.close();
		}
	}

	void LoadNextText() {
		++index %= (int)texts.size();
	}

	void SetIndex(int n) { index = n; }
	inline int GetIndex() const { return index; }

	std::size_t GetTextSize() const {
		return texts[index].size();
	}

	Texts GetTexts() const { return texts; }

	void DrawTextHelp(const sf::Vector2f& pos, sf::RenderWindow& window) {

		float textPos = pos.y;

		for (int i = 0; i < (int)texts[index].size(); i++) {
			RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), pos.x, pos.y + i * 16.0f, texts[index][i], sf::Color::White, 16);
		}
	}
};

bool isEditorRunState = false;
bool isMusicPlaying = true;

class GameState {
public:
	sf::Vector2u windowSize;
	bool isStateChanged;
	enum class State {
		Menu = 0,
		Play = 1,
		Editor = 2,
		LevelScene = 3,
		Quit = 4
	} state;

	sf::Music music; //Background music

	GameState(const sf::Vector2u& size)
		: windowSize(size) {
		isStateChanged = false;
	}

	void SetState(const State& newState) {
		state = newState;
		isStateChanged = true;
	}

	virtual void Input() {}
	virtual void ManageEvent(sf::Event, sf::Vector2f) {}
	virtual void Logic(float) = 0;
	virtual void Render(sf::RenderWindow&) = 0;
	void MusicClear() {
		music.stop();
		sf::sleep(sf::seconds(2.0f));
	}

	static bool KeyPress(sf::Keyboard::Key key) {
		return sf::Keyboard::isKeyPressed(key);
	};

	static bool MouseButton(sf::Mouse::Button button) {
		return sf::Mouse::isButtonPressed(button);
	}
};

int scenesType = 0;
class SceneState : public GameState {
private:
	sf::Sprite sceneSprite;
	TextManager textManager;
	sf::RectangleShape box;

	int nLevelScenes, nCurrentLevelScene;

	const std::vector<std::string> sceneNames = {
		"scene01",
		"scene02",
		"scene03",
		"scene04",
		"scene05",
		"scene06",
		"scene07",
		"scene08",
		"scene09",
		"scene11",
		"scene12",
		"scene21",
		"scene22",
		"scene31",
		"scene32",
		"scene33",
		"scene34"
	};
public:
	SceneState(const sf::Vector2u& size)
		: GameState(size) {

		textManager.LoadTexts("files/sceneTexts.txt");
		nLevelScenes = 9;
		nCurrentLevelScene = 0;

		switch(scenesType) {
		case 1:
			nLevelScenes = 9 + 2;
			nCurrentLevelScene = 9;
			textManager.SetIndex(nCurrentLevelScene);
			break;
		case 2:
			nLevelScenes = 11 + 2;
			nCurrentLevelScene = 11;
			textManager.SetIndex(nCurrentLevelScene);
			break;
		case 3:
			nLevelScenes = 13 + 4;
			nCurrentLevelScene = 13;
			textManager.SetIndex(nCurrentLevelScene);
			break;
		}
		
		box.setSize({ (float)size.x, textManager.GetTextSize() * 16.0f });
		box.setFillColor(sf::Color(0, 0, 0, 100));
		box.setOutlineColor(sf::Color(50, 50, 50));
		box.setOutlineThickness(-2.0f);

		sceneSprite.setTexture(AssetHolder::Get().GetTexture(sceneNames[nCurrentLevelScene]));
	}

	void Input() override {}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		switch (e.type) {
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				nCurrentLevelScene++;
				if (nCurrentLevelScene >= nLevelScenes) {
					if (scenesType < 3) SetState(State::Play);
					else {
						SetState(State::Menu);
						scenesType = 0;
					}
				}
				else {
					textManager.LoadNextText();
					box.setSize({ (float)windowSize.x, textManager.GetTextSize() * 16.0f });
					sceneSprite.setTexture(AssetHolder::Get().GetTexture(sceneNames[nCurrentLevelScene]));
				}
				break;
			}
			break;
		}
	}

	void Logic(float dt) override {}

	void Render(sf::RenderWindow& window) override {
		window.draw(sceneSprite);
		window.draw(box);
		textManager.DrawTextHelp({ 0.0f, 0.0f }, window);
	}
};

class EditorState : public GameState {
private:
	sf::Vector2f mousePosition, playerPos;
	sf::Sprite tileSet, tile;
	sf::RectangleShape tilePixel, selectedTile;
	std::list<Tile> levelTiles;
	const std::string& tileCharacters = "1239W8#4BW765BT"; //Tile Characters
	int index, tileSetWidth, tileSetOffset;

	bool isTileSetDrawn, isKeyPressed;
	int nLineWidth, nLineHeight; //Max lines along with and height

	void DrawGrid(sf::RenderWindow& window, float x1, float y1, float x2, float y2) {
		for (int i = 0; i < nLineWidth; i++) {
			DrawLine(window, x1, y1 + i * pixelSize, x2, y1 + i * pixelSize);
		}

		for (int i = 0; i < nLineHeight; i++) {
			DrawLine(window, x1 + i * pixelSize, y1, x1 + i * pixelSize, y2);
		}
	}

	void Run() {

		isEditorRunState = true;

		Level editorLevel = Level::LoadLevel(levelTiles, (unsigned)nLineHeight + 1, (unsigned)nLineWidth + 1);
		for (uint32_t i = 1; i < editorLevel.GetHeight() - 1; i++) {
			for (uint32_t j = 1; j < editorLevel.GetWidth() - 1; j++) {

				switch (editorLevel.GetCharacter(j, i)) {
				case 'W':
				case 'B':
				case 'T':
				case 'P':
					editorLevel.SetCharacter(j, i, '#');
					break;
				}
			}
		}

		editorLevel.SaveLevel("EditorLevel.lvl");

		Level editorLevelItemMap = Level::LoadLevel(levelTiles, (unsigned)nLineHeight + 1, (unsigned)nLineWidth + 1);
		for (uint32_t i = 1; i < editorLevelItemMap.GetHeight() - 1; i++) {
			for (uint32_t j = 1; j < editorLevelItemMap.GetWidth() - 1; j++) {

				if ((int)(playerPos.x / pixelSize) == j && (int)(playerPos.y / pixelSize) == i) {
					editorLevelItemMap.SetCharacter(j, i, 'P');
				}

				switch (editorLevelItemMap.GetCharacter(j, i)) {
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					editorLevelItemMap.SetCharacter(j, i, '#');
					break;
				}
			}
		}

		editorLevelItemMap.SaveLevel("EditorLevelItemMap.lvl");

		editorLevel.PrintLevel();
		editorLevelItemMap.PrintLevel();

		SetState(State::Play);
	}
public:
	EditorState(const sf::Vector2u& size)
		: GameState(size) {
		tileSet.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		tileSet.setTextureRect(sf::IntRect(0, 0, 5 * (int)pixelSize, 3 * (int)pixelSize));
		tileSet.setPosition((float)size.x - 5.0f * pixelSize, 0.0f);
		tile.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		tile.setTextureRect(sf::IntRect((int)pixelSize, (int)pixelSize, (int)pixelSize, (int)pixelSize));

		tilePixel.setSize({ pixelSize, pixelSize });
		tilePixel.setFillColor(sf::Color(0, 100, 200, 100));

		selectedTile.setSize({ pixelSize, pixelSize });
		selectedTile.setFillColor(sf::Color(200, 200, 100, 100));

		music.openFromFile("files/sounds/menuBg.wav");
		music.setLoop(true);
		music.play();

		isTileSetDrawn = true;
		isKeyPressed = false;

		if (isEditorRunState) {
			Level level = Level::LoadLevel("files/levels/EditorLevel.lvl");
			for (uint32_t i = 1; i < level.GetHeight() - 1; i++) {
				for (uint32_t j = 1; j < level.GetWidth() - 1; j++) {
					if (level.GetCharacter(j, i) == '.') {
						continue;
					}
					else {
						levelTiles.emplace_back(j, i, level.GetCharacter(j, i));
					}
				}
			}

			level = Level::LoadLevel("files/levels/EditorLevelItemMap.lvl");

			for (uint32_t i = 1; i < level.GetHeight() - 1; i++) {
				for (uint32_t j = 1; j < level.GetWidth() - 1; j++) {
					switch (level.GetCharacter(j, i)) {
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '#':
					case '.':
						continue;
						break;
					case 'P':
						playerPos = { j * pixelSize, i * pixelSize };
						break;
					}

					levelTiles.emplace_back(j, i, level.GetCharacter(j, i));
				}
			}
		}

		nLineWidth = 15;
		nLineHeight = 11;

		tileSetWidth = 5;
		tileSetOffset = 11;

		playerPos = { -pixelSize, -pixelSize };
		index = 0;
	}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		mousePosition = mousePos;

		switch (e.type) {
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::LShift:
				isTileSetDrawn = !isTileSetDrawn;
				break;
			case sf::Keyboard::LControl:
				isKeyPressed = true;
				break;
			case sf::Keyboard::R:
				if (isKeyPressed) {
					Run();
				}
				break;
			case sf::Keyboard::Escape:
				state = State::Menu;
				isStateChanged = true;
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch (e.key.code) {
			case sf::Keyboard::LControl:
				isKeyPressed = false;
				break;
			}
			break;
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				if (isTileSetDrawn) {
					auto [x, y] = (sf::Vector2i)(mousePosition / pixelSize);

					if (tileSet.getGlobalBounds().contains(x * pixelSize, y * pixelSize)) {
						selectedTile.setPosition(x * pixelSize, y * pixelSize);
						index = y * tileSetWidth + x - tileSetOffset;
					}
				}
				break;
			}
			break;
		}
	}

	void Input() override {

		if (MouseButton(sf::Mouse::Left)) {
			auto [x, y] = (sf::Vector2i)(mousePosition / pixelSize);

			if (x > 0 && y > 0 && x < nLineHeight && y < nLineWidth) {

				if (isKeyPressed) {
					playerPos = { x * pixelSize, y * pixelSize };
				}
				else {

					bool isTile = false;
					for (auto& tiles : levelTiles) {
						if (tiles.x == x && tiles.y == y) {
							tiles.tileCharacter = tileCharacters[index];
							isTile = true;
						}
					}

					if (!isTile) {
						levelTiles.emplace_back(x, y, tileCharacters[index]);
					}
				}
			}
		}

		if (MouseButton(sf::Mouse::Right)) {
			auto [x, y] = (sf::Vector2i)(mousePosition / pixelSize);

			if (x > 0 && y > 0 && x < nLineHeight && y < nLineWidth) {
				for (auto it = levelTiles.begin(); it != levelTiles.end();) {
					if (it->x == x && it->y == y) {
						it = levelTiles.erase(it);
						break;
					}
					else {
						it++;
					}
				}
			}
		}
	}

	void Logic(float dt) override {

		auto [x, y] = (sf::Vector2i)(mousePosition / pixelSize);

		tilePixel.setPosition(x * pixelSize, y * pixelSize);
	}

	void SetRect(int x, int y) {
		tile.setTextureRect(sf::IntRect(x * (int)pixelSize, y * (int)pixelSize, (int)pixelSize, (int)pixelSize));
	}

	void Render(sf::RenderWindow& window) {

		DrawGrid(window, pixelSize, pixelSize, nLineHeight * pixelSize, nLineWidth * pixelSize);

		for (auto& tiles : levelTiles) {
			switch (tiles.tileCharacter) {
			case '#':
				SetRect(1, 1);
				break;
			case '1':
				SetRect(0, 0);
				break;
			case '2':
				SetRect(1, 0);
				break;
			case '3':
				SetRect(2, 0);
				break;
			case '4':
				SetRect(2, 1);
				break;
			case '5':
				SetRect(2, 2);
				break;
			case '6':
				SetRect(1, 2);
				break;
			case '7':
				SetRect(0, 2);
				break;
			case '8':
				SetRect(0, 1);
				break;
			case '9':
				SetRect(3, 0);
				break;
			case 'W':
				SetRect(4, 0);
				break;
			case 'B':
				SetRect(3, 1);
				break;
			case 'T':
				SetRect(4, 2);
				break;
			case '.':
				continue;
				break;
			}

			tile.setPosition(tiles.x * pixelSize, tiles.y * pixelSize);
			window.draw(tile);
		}

		if (isTileSetDrawn) {
			window.draw(tileSet);
			window.draw(selectedTile);
		}

		window.draw(tilePixel);

		RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), playerPos.x, playerPos.y, "P");
		RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), 0.0f, 0.0f, "Place Player - Ctrl + LMB\nRun - Ctrl + R", sf::Color::White, 16);
	}
};

//Transition between levels
class Transition {
private:
	sf::RectangleShape screen;

	int alpha;

	bool isTransition;
	int colorState;
public:
	bool isLoadNextLevel;
	Transition() {}
	Transition(const sf::Vector2f& size) {

		alpha = 1;
		isTransition = false;

		colorState = 1;
		isLoadNextLevel = true;

		screen.setSize(size);
		screen.setFillColor(sf::Color(0, 0, 0, alpha));
	}

	void Logic() {
		if (alpha > 250) {
			colorState = -1;
			isLoadNextLevel = false;
		}

		alpha += 5 * colorState;

		if (alpha <= 0) {
			isTransition = false;
			colorState = 1;
			alpha = 1;
		}
	}

	void Render(sf::RenderWindow& window) {
		screen.setFillColor(sf::Color(0, 0, 0, alpha));
		window.draw(screen);
	}

	inline bool GetTransition() const { return isTransition; }
	void SetTransition(bool state) { isTransition = state; }

	inline int GetColorState() const { return colorState; }
};

class MenuState : public GameState {
private:
	gui::SpriteButton playButton, editorButton, quitButton;
	sf::Sprite background, musicToggler;

	Transition transitionEffect;

	bool isBackgroundDrawn;
	int button;
public:
	MenuState(const sf::Vector2u& size)
		: GameState(size) {
		playButton = gui::SpriteButton(0, 0, 200, 64, { 150.0f, 192.0f });
		playButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		editorButton = gui::SpriteButton(0, 2, 200, 64, { 150.0f, 272.0f });
		editorButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		quitButton = gui::SpriteButton(0, 1, 200, 64, { 150.0f, 352.0f });
		quitButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		background.setTexture(AssetHolder::Get().GetTexture("menuBackground"));
		isBackgroundDrawn = true;

		musicToggler.setTexture(AssetHolder::Get().GetTexture("soundToggler"));
		musicToggler.setTextureRect(sf::IntRect(0, 0, 64, 64));
		musicToggler.setPosition(0.0f, (float)size.y - 64);

		transitionEffect = Transition((sf::Vector2f)size);
		button = -1;

		music.openFromFile("files/sounds/menuBg.wav");
		music.setLoop(true);
		music.play();
	}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		playButton.Logic(0, 0, e, mousePos);
		quitButton.Logic(0, 1, e, mousePos);
		editorButton.Logic(0, 2, e, mousePos);
	
		switch (e.type) {
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				auto [x, y] = sf::Vector2f((float)e.mouseButton.x, (float)e.mouseButton.y);
				if (musicToggler.getGlobalBounds().contains(x, y)) {
					isMusicPlaying = !isMusicPlaying;
					if (!isMusicPlaying) {
						music.pause();
					}
					else {
						music.play();
					}
				}

				transitionEffect.SetTransition(true);
				break;
			}
			break;
		}
	}

	void Logic(float dt) override {
		 
		if (transitionEffect.GetTransition()) {
			transitionEffect.Logic();
			if (!transitionEffect.isLoadNextLevel) {
				if (isBackgroundDrawn) isBackgroundDrawn = false;
				else {
					switch (button) {
					case 1:
						SetState(State::LevelScene);
						break;
					case 2:
						SetState(State::Editor);
						break;
					}
				}
				transitionEffect.isLoadNextLevel = true;
			}
		}

		if (isBackgroundDrawn) return;

		if (playButton.GetIsPressed()) {
			transitionEffect.SetTransition(true);
			button = 1;

			isEditorRunState = false;
			music.stop();
		}
		else if (quitButton.GetIsPressed()) {
			SetState(State::Quit);
		}
		else if (editorButton.GetIsPressed()) {
			transitionEffect.SetTransition(true);
			button = 2;
		}
	}

	void Render(sf::RenderWindow& window) override {
		if (isBackgroundDrawn) {
			window.draw(background);
		}
		else {
			playButton.Render(window);
			quitButton.Render(window);
			editorButton.Render(window);
			musicToggler.setTextureRect(sf::IntRect((int)!isMusicPlaying * 64, 0, 64, 64));
			window.draw(musicToggler);
		}

		if (transitionEffect.GetTransition()) {
			transitionEffect.Render(window);
		}
	}
};

class Box {
private:
	sf::Vector2f resetPosition, position;
	bool isValue;
public:
	Box() {}
	Box(const sf::Vector2f& pos)
		: resetPosition(pos), position(pos) {
		isValue = true;
	}

	void Logic(Level& level, const sf::Vector2f& playerPos, const sf::Vector2f& direction) {

		auto [x, y] = (sf::Vector2i)(position / pixelSize);

		if (position == playerPos) {
			if (level.GetCharacter(x + (int)direction.x, y + (int)direction.y) == '#') {
				position += direction * pixelSize;
			}
			else if (level.GetCharacter(x + (int)direction.x, y + (int)direction.y) == '.') {
				level.SetCharacter(x + (int)direction.x, y + (int)direction.y, '#');
				isValue = false;
			}
		}
	}

	void Reset() { 
		position = resetPosition; 
		isValue = true;
	}
	inline bool GetIsValue() const { return isValue; }
	void SetIsValue(bool state) { isValue = state; }
	inline sf::Vector2f GetPosition() const { return position; }
};

class ToggleTile {
private:
	sf::Vector2f position;
	bool isTileActive;
public:
	ToggleTile() {}
	ToggleTile(const sf::Vector2f& pos)
		: position(pos) {
		isTileActive = false;
	}

	void Logic(const std::vector<Box>& boxes) {
		for (auto& box : boxes) {

			isTileActive = false;

			if (position == box.GetPosition()) {
				isTileActive = true;
				break;
			}
		}
	}

	void Reset() { isTileActive = false; }
	void SetPosition(const sf::Vector2f& pos) { position = pos; }
	inline bool GetIsTileActive() const { return isTileActive; }
	void SetIsTileActive(bool isActive) { isTileActive = isActive; }
	inline sf::Vector2f GetPosition() const { return position; }
};

class Opponent {
private:
	sf::Vector2f resetPos, pos;
	std::vector<sf::Vector2i> movePositions;
	std::vector<std::string> movePos; //Read from a text file
	sf::RectangleShape box;
	int nLevel, index, direction;
public:
	Opponent() {
		nLevel = 0;
		index = 0;
		direction = 1;

		box.setSize({ pixelSize, pixelSize });
		box.setFillColor(sf::Color::Magenta);
		box.setOutlineColor(sf::Color(255, 100, 100));
		box.setOutlineThickness(-2.0f);
	}

	void SetResetPos(const sf::Vector2f& initPos) {
		resetPos = initPos;
		pos = initPos;

		box.setPosition(resetPos);
	}

	void Reset() {
		index = 0;
		direction = 1;
		pos = resetPos;
		box.setPosition(resetPos);
	}

	void ClearMovePositions() {
		movePositions.clear();
	}

	void LoadMovePositions(const std::string& filename) {
		std::ifstream reader(filename);

		if (reader.is_open()) {

			while (!reader.eof()) {
				std::string str;
				reader >> str;
				movePos.push_back(str);
			}

			reader.close();
		}
	}

	void IntrepretPositions() {
		const std::string& directions = movePos[nLevel];

		for (char c : directions) {
			switch (c) {
			case '0': //Right
				movePositions.push_back({ 1, 0 });
				break;
			case '1': //Down
				movePositions.push_back({ 0, 1 });
				break;
			case '2': //Left
				movePositions.push_back({ -1, 0 });
				break;
			case '3': //Up
				movePositions.push_back({ 0, -1 });
				break;
			}
		}

		nLevel++;
		if (nLevel >= (int)movePos.size()) nLevel = (int)(movePos.size());
	}

	void Move(const sf::Vector2f& playerPos) {

		sf::Vector2f newPos = pos + (sf::Vector2f)movePositions[index] * pixelSize * (float)direction;
		if (newPos != playerPos) {
			pos += (sf::Vector2f)movePositions[index] * pixelSize * (float)direction;
			index += direction;
		}

		if (index >= (int)movePositions.size() || index < 0) {
			if (index >= (int)movePositions.size()) index = (int)(movePositions.size() - 1);
			else if (index < 0) index = 0;
			direction = -direction;
		}

		box.setPosition(pos);
	}

	void Render(sf::RenderWindow& window) {
		window.draw(box);
	}

	inline sf::Vector2f GetPosition() const { return pos; }
};

class Player {
private:
	sf::Vector2f resetPlayerPos, playerPos;
	sf::Sprite sprite;
	std::vector<std::pair<sf::Vector2i, int>> movePositions;
	std::vector<sf::Vector2i> changedTiles;
	//std::vector<int> rotations;
	int index, nMoves, direction;
	bool isIndex, isWin; //isIndex -> index == (movePositions.size - 1) & isWin -> has player won a level

	void IntrepretMovement(const std::string& text) {
		if (text == "fd") { //Move forward
			switch (direction) {
			case 0: //Right
				AddMovePosition(1, 0);
				break;
			case 1: //Down
				AddMovePosition(0, 1);
				break;
			case 2: //Left
				AddMovePosition(-1, 0);
				break;
			case 3: //Up
				AddMovePosition(0, -1);
				break;
			}
		}
		else if (text == "bk") { //Move backward
			switch (direction) {
			case 0: //Right
				AddMovePosition(-1, 0);
				break;
			case 1: //Down
				AddMovePosition(0, -1);
				break;
			case 2: //Left
				AddMovePosition(1, 0);
				break;
			case 3: //Up
				AddMovePosition(0, 1);
				break;
			}
		}
	}

	void IntrepretRotation(const std::string& text) {
		if (text == "rt" || text == "right") {
			direction++;
			if (direction > 3) direction = 0;
		}
		else if (text == "lt" || text == "left") {
			direction--;
			if (direction < 0) direction = 3;
		}
	}

	std::size_t IntrepretLoop(const Level& level, bool& isRun, const std::vector<std::string>& strings, int nStartPoint, int nLoop) {
		std::vector<std::string> stringsToLoop;
		bool isLoopPoint = false;
		int conditionalPoint = 0;

		for (int i = nStartPoint; i < (int)strings.size(); i++) {

			auto textForLoop = ToWords(strings[i]);
			if (textForLoop.size() == 0) continue;

			if (textForLoop[0] == "closeloop" || textForLoop[0] == "end") { //Openloop close
				isLoopPoint = true;
				break;
			}

			if (textForLoop[0] == "if") conditionalPoint = i;

			else if (textForLoop[0] == "openloop" || textForLoop[0] == "repeat") {
				continue;
			}

			stringsToLoop.push_back(strings[i]);
		}

		if (!isLoopPoint) return 0;

		for (int n = 0; n < nLoop; n++) {
			int value = 0;

			for (int i = 0; i < (int)stringsToLoop.size(); i++) {

				int stringIndex = i + value;
				if (stringIndex > (int)(stringsToLoop.size() - 1)) break;

				auto text = ToWords(stringsToLoop[stringIndex]);
				if (text.size() == 0) continue;

				if (text[0] == "move") {
					if (text.size() >= 2) {
						IntrepretMovement(text[1]);
					}
				}
				else if (text[0] == "turn") {
					if (text.size() >= 2) {
						IntrepretRotation(text[1]);
						//rotations.push_back(direction);
					}
				}
				else if (text[0] == "if") {
					int nLines = (int)IntrepretCheckConditional(level, strings, conditionalPoint);
					value += nLines;
				}
			}
		}

		return stringsToLoop.size();
	}

	std::size_t IntrepretCheckConditional(const Level& level, const std::vector<std::string>& strings, int nStartPoint) {
		std::vector<std::string> first, second;

		bool isConditionSatisfied = true;
		bool isFirstComplete = false;

		for (int i = nStartPoint; i < (int)strings.size(); i++) {
			auto text = ToWords(strings[i]);
			if (text.size() == 0) continue;

			if (text.size() >= 3) {
				if (text[0] == "if" && text[1] == "fd" && text[2] == "empty") {
					int x = (int)(playerPos.x / pixelSize), y = (int)(playerPos.y / pixelSize);
					for (const auto& pos : movePositions) {
						x += pos.first.x;
						y += pos.first.y;
					}

					if (direction == 0) { isConditionSatisfied = (bool)(level.GetCharacter((uint32_t)x + 1, (uint32_t)y) == '#'); }
					else if (direction == 1) { isConditionSatisfied = (bool)(level.GetCharacter((uint32_t)x, (uint32_t)y + 1) == '#'); }
					else if (direction == 2) { isConditionSatisfied = (bool)(level.GetCharacter((uint32_t)x - 1, (uint32_t)y) == '#'); }
					else if (direction == 3) { isConditionSatisfied = (bool)(level.GetCharacter((uint32_t)x, (uint32_t)y - 1) == '#'); }

					continue;
				}
			}

			if (text[0] == "else" || strings[i] == "else") {
				isFirstComplete = true;
				continue;
			}

			if (text[0] == "closeif" || text[0] == "endif") break;

			if (!isFirstComplete) first.push_back(strings[i]);
			else second.push_back(strings[i]);
		}

		std::size_t size = isConditionSatisfied ? first.size() : second.size();
		for (int i = 0; i < (int)size; i++) {
			auto text = ToWords(isConditionSatisfied ? first[i] : second[i]);
			if (text.size() == 0) continue;

			if (text[0] == "move") IntrepretMovement(text[1]);
			else if (text[0] == "turn") IntrepretRotation(text[1]);
		}

		return first.size() + second.size() + 3;
	}
public:
	Player() {
		index = 0;
		nMoves = 0;
		isIndex = false;
		isWin = false;

		sprite.setTextureRect(sf::IntRect(direction * (int)pixelSize, 0, (int)pixelSize, (int)pixelSize));
	}

	void LoadSprite(const sf::Texture& texture) {
		sprite.setTexture(texture);
	}

	void Move(std::vector<Box>& boxes, Opponent& o, Level& level) {
		//Player coords in level
		auto [x, y] = (sf::Vector2i)(playerPos / pixelSize);

		if (movePositions.size() > 0) {

			bool isMove = true;

			//New Position of player in level
			sf::Vector2i playerToLevelIndex = { x + movePositions[index].first.x, y + movePositions[index].first.y };
			sf::Vector2f newPlayerPos = (sf::Vector2f)playerToLevelIndex * pixelSize;

			for (auto& box : boxes) {
				if (box.GetIsValue()) {

					char boxChar = level.GetCharacter(playerToLevelIndex.x + movePositions[index].first.x,
						playerToLevelIndex.y + movePositions[index].first.y);

					if (newPlayerPos == box.GetPosition()) {
						if (boxChar == '.') {
							level.SetCharacter(playerToLevelIndex.x + movePositions[index].first.x,
								playerToLevelIndex.y + movePositions[index].first.y, '#');

							changedTiles.emplace_back(playerToLevelIndex.x + movePositions[index].first.x,
								playerToLevelIndex.y + movePositions[index].first.y);

							box.SetIsValue(false);
							isMove = false;
						}
						if (boxChar != '#') {
							isMove = false;
						}
					}
				}
			}

			if (newPlayerPos == o.GetPosition()) {
				isMove = false;
				nMoves++;
			}

			if (isMove && level.GetCharacter(playerToLevelIndex.x, playerToLevelIndex.y) == '#') {
				nMoves++;
				sprite.setTextureRect(sf::IntRect(movePositions[index].second * (int)pixelSize, 0, (int)pixelSize, (int)pixelSize));
				playerPos += sf::Vector2f(movePositions[index].first.x * pixelSize, movePositions[index].first.y * pixelSize);
			}

			index++;
		}
	}

	void Run(const Level& level, bool& isRun, const std::vector<std::string>& strings) {
		movePositions.clear();
		//rotations.clear();

		int value = 0;
		for (int i = 0; i < (int)strings.size(); i++) {

			//StringIndex ensures that looping & conditional strings are not re-read
			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) break;

			int x = 0, y = 0;
			for (const auto& pos : movePositions) {
				x += pos.first.x;
				y += pos.first.y;
			}

			if (level.GetCharacter((uint32_t)x, (uint32_t)y) == 'W') {
				isRun = true;
				break;
			}

			auto text = ToWords(strings[stringIndex]);
			if (text.size() == 0) continue;

			if (text[0] == "move") {
				if (text.size() >= 2) {
					IntrepretMovement(text[1]);
				}
			}

			if (text[0] == "turn") {
				if (text.size() >= 2) {
					IntrepretRotation(text[1]);
					//rotations.push_back(direction);
				}
			}

			if (text[0] == "if") {
				int nLines = (int)IntrepretCheckConditional(level, strings, stringIndex) - 1;
				value += nLines;
			}

			if (text[0] == "openloop" || text[0] == "repeat") {
				if (text.size() >= 2) {
					int n = std::stoi(text[1]);

					//nLines -> number of lines in loop
					int nLines = (int)IntrepretLoop(level, isRun, strings, stringIndex, n);
					value += nLines;
				}
			}
		}
	}

	void Logic(Level& itemMap, bool& isRun, bool isWinTileActive, const std::vector<ToggleTile>& tiles) {
		auto [x, y] = (sf::Vector2i)(playerPos / pixelSize);
		auto [w, h] = sf::Vector2u(itemMap.GetWidth(), itemMap.GetHeight());

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x > (int)(w - 1)) x = (int)(w - 1);
		if (y > (int)(h - 1)) y = (int)(h - 1);

		isWin = false;

		switch (itemMap.GetLevel()[y][x]) {
		case 'A':
			itemMap.SetCharacter((unsigned)x, (unsigned)y, '#');
			break;
		case 'S':
			movePositions.clear();
			Reset();
			break;
		case 'W':
			bool isTileIntrepreted = false;
			int n = 0;
			for (auto& tile : tiles) {
				isTileIntrepreted = true;
				if (tile.GetIsTileActive()) {
					n++;
				}
				else {
					break;
				}
			}

			if ((n == (int)(tiles.size()) && !isWinTileActive) ||
				(!isTileIntrepreted && isWinTileActive)) {
				isWin = true;
			}

			break;
		}

		if (index > (int)(movePositions.size() - 1)) {
			isRun = false;
			isIndex = true;
		}

		sprite.setPosition(playerPos);
	}

	void AddMovePosition(int dirX, int dirY) {
		movePositions.push_back({ { dirX, dirY }, direction });
	}

	void SetPosition(const sf::Vector2f& pos) {
		playerPos = pos;
		resetPlayerPos = pos;
	}

	void ResetWin() { isWin = false; }
	void Reset() {
		nMoves = 0;
		isIndex = false;
		index = 0;
		ResetWin();
		SetPosition(resetPlayerPos);
		direction = 0;
		sprite.setTextureRect(sf::IntRect(direction * (int)pixelSize, 0, (int)pixelSize, (int)pixelSize));
	}

	void Render(sf::RenderWindow& window) {
		window.draw(sprite);
	}

	inline int GetNMoves() const { return nMoves; }
	inline bool GetIsWin() const { return isWin; }
	inline bool GetIsIndex() const { return isIndex; }
	inline sf::Vector2f GetPosition() const { return playerPos; }
	std::vector<sf::Vector2i> GetChangedTiles() const { return changedTiles; }

	std::vector<std::pair<sf::Vector2i, int>> GetMovePositions() const { return movePositions; }
	sf::Vector2f GetCurrentMovePosition() {
		int posIndex = index;
		if (posIndex > (int)(movePositions.size() - 1)) {
			posIndex = (int)(movePositions.size() - 1);
		}
		return (sf::Vector2f)movePositions[posIndex].first;
	}
};

typedef Level ItemMap;
class LevelManager {
private:

	struct LevelData {
		std::string level, itemMap;

		LevelData(const std::string& levelStr, const std::string& itemMapStr)
			: level(levelStr), itemMap(itemMapStr) {}
	};

	std::vector<LevelData> strings;
	std::string folder, extension;

	int index;

	Level level;
	ItemMap itemMap;
	bool isWinTileActive;

	void LoadLevel(std::string fileExtension = ".lvl") {
		level = Level::LoadLevel(folder + strings[index].level + fileExtension);
		itemMap = Level::LoadLevel(folder + strings[index].itemMap + fileExtension);
	}
public:
	LevelManager() {
		folder = "files/levels/";
		extension = ".txt";
		index = 0;

		LoadLevelStrings("LevelStrings");
	}

	void LoadLevelStrings(const std::string& filename) {
		std::ifstream reader(folder + filename + extension);

		if (reader.is_open()) {

			while (!reader.eof()) {
				std::string levelName, itemMapName;
				reader >> levelName >> itemMapName;
				strings.emplace_back(levelName, itemMapName);
			}

			reader.close();
		}

		if ((int)strings.size() > 0) {
			LoadLevel();
		}
	}

	void LoadLevelFromFile(const std::string& filepath) {
		level = Level::LoadLevel(filepath);
	}

	void LoadItemMap(const std::string& filepath) {
		itemMap = ItemMap::LoadLevel(filepath);
	}

	Level& GetLevel() { return level; }
	ItemMap& GetItemMap() {
		return itemMap;
	}
	inline int GetIndex() const { return index; }
	void SetIndex(int n) { 
		index = n; 
		LoadLevel();
	}

	void ResetIndex() { index = 0; }
	void LoadNextLevel() {
		++index %= (int)strings.size();
		LoadLevel();
	}
};

class PauseUI {
private:
	sf::RectangleShape pauseScreen;
	sf::Sprite musicToggler;

	bool isPaused;
public:
	PauseUI() {}
	PauseUI(const sf::Vector2u& size) {
		pauseScreen.setSize((sf::Vector2f)size);
		pauseScreen.setFillColor(sf::Color(255, 255, 255, 100));
		
		musicToggler.setTexture(AssetHolder::Get().GetTexture("soundToggler"));
		musicToggler.setTextureRect(sf::IntRect(0, 0, 64, 64));
		musicToggler.setPosition(0.0f, (float)size.y - 64);

		isPaused = false;
	}

	void Render(sf::RenderWindow& window) {
		if (isPaused) {
			window.draw(pauseScreen);
			RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), 
				pauseScreen.getSize().x / 2.0f - 100.0f, pauseScreen.getSize().y / 2.0f, "Game Paused", sf::Color::Yellow);
			RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), 
				pauseScreen.getSize().x / 2.0f - 180.0f, pauseScreen.getSize().y / 2.0f + 80.0f, "Press Q to goto Menu", sf::Color(255, 100, 0));
		
			musicToggler.setTextureRect(sf::IntRect((int)!isMusicPlaying * 64, 0, 64, 64));
			window.draw(musicToggler);
		}
	}

	void Logic(sf::Event e) {
		switch (e.type) {
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				if (isPaused && GetIsMusicTogglerPressed((float)e.mouseButton.x, (float)e.mouseButton.y)) {
					isMusicPlaying = !isMusicPlaying;
				}
				break;
			}
			break;
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::Escape:
				isPaused = !isPaused;
				break;
			}
			break;
		}
	}

	inline bool GetIsMusicTogglerPressed(float x, float y) const {
		return musicToggler.getGlobalBounds().contains(x, y);
	}

	inline int GetIsPaused() const { return isPaused; }
	void SetPauseState(bool state) { isPaused = state; }
};

class PlayState : public GameState {
private:
	LevelManager levelManager; //Level Manager
	TextManager textManager; //Text Manager
	sf::RectangleShape textBox;

	Transition transitionScreen; //Transition between levels
	PauseUI pauseUI;
	int nPress;

	sf::Sprite spriteTile, background;

	Player player;
	Opponent opponent;

	std::vector<Box> boxes;
	std::vector<ToggleTile> tiles;

	TextWindow textWindow;
	gui::SpriteButton runButton, clearButton;
	sf::Text text;

	bool isRun, isButtonPressable, isHowToPlay, isToggleTileInLevel, isOpponentInLevel, isKeyPressed;

	sf::Clock clock;
	int t, delay;

	void Initialize() {

		boxes.clear();
		tiles.clear();
		player.Reset();
		opponent.Reset();
		opponent.ClearMovePositions();
		opponent.SetResetPos({ -pixelSize, -pixelSize });
		isToggleTileInLevel = false;

		ItemMap& map = levelManager.GetItemMap();
		isOpponentInLevel = false;

		for (uint32_t i = 1; i < map.GetHeight() - 1; i++) {
			for (uint32_t j = 1; j < map.GetWidth() - 1; j++) {
				char c = map.GetCharacter(j, i);

				switch (c) {
				case 'P': //Player Position
					player.SetPosition({ j * pixelSize, i * pixelSize });
					map.SetCharacter(j, i, '#');
					break;
				case 'B': //Box Position
					boxes.push_back(Box({ j * pixelSize, i * pixelSize }));
					map.SetCharacter(j, i, '#');
					break;
				case 'O': //Opponent Position
					opponent.SetResetPos({ j * pixelSize, i * pixelSize });
					map.SetCharacter(j, i, '#');
					isOpponentInLevel = true;
					break;
				case 'T': //ToggleTile Position
					tiles.push_back(ToggleTile({ j * pixelSize, i * pixelSize }));
					map.SetCharacter(j, i, '#');
					isToggleTileInLevel = true;
					break;
				}
			}
		}
	
		opponent.IntrepretPositions();
	}
public:
	PlayState(const sf::Vector2u& size)
		: GameState(size) {

		textWindow = TextWindow({ (float)size.x - 150.0f, 0.0f }, { 150.0f, (float)size.y });
		textWindow.SetFont(AssetHolder::Get().GetFont("lucidaConsole"));

		spriteTile.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		player.LoadSprite(AssetHolder::Get().GetTexture("player"));
		
		transitionScreen = Transition((sf::Vector2f)size);
		pauseUI = PauseUI(size);
		nPress = 0;

		if (isEditorRunState) {
			background.setTexture(AssetHolder::Get().GetTexture("background"));
			levelManager.LoadLevelFromFile("files/levels/EditorLevel.lvl");
			levelManager.LoadItemMap("files/levels/EditorLevelItemMap.lvl");
		}
		else {
			if (scenesType == 0) background.setTexture(AssetHolder::Get().GetTexture("howToPlay"));
		}

		textManager.LoadTexts("files/texts.txt");
		textBox.setSize({ (float)windowSize.x, (float)textManager.GetTextSize() * 16.0f });
		textBox.setFillColor(sf::Color(0, 0, 0, 100));
		textBox.setOutlineColor(sf::Color(50, 50, 50));
		textBox.setOutlineThickness(-2.0f);

		if (!isEditorRunState) {

			levelManager.SetIndex(19);
			textManager.SetIndex(19);

			switch (scenesType) {
			case 1:
				levelManager.SetIndex(10);
				textManager.SetIndex(10);
				break;
			case 2:
				levelManager.SetIndex(19);
				textManager.SetIndex(19);
				break;
			case 3:
				levelManager.SetIndex(20);
				textManager.SetIndex(20);
				break;
			}
		}

		opponent.LoadMovePositions("files/opponentPos.txt");
		Initialize();

		delay = 100; //Milliseconds

		isRun = true;
		player.Run(levelManager.GetLevel(), isRun, textWindow.GetStrings());

		isKeyPressed = false;
		isButtonPressable = true;
		if (scenesType == 0) isHowToPlay = true; //Is How To Play background rendered
		else background.setTexture(AssetHolder::Get().GetTexture("background"));

		runButton = gui::SpriteButton(0, 0, 64, 64, { 0.0f, size.y - 70.0f });
		runButton.LoadSprite(AssetHolder::Get().GetTexture("buttons"));
		clearButton = gui::SpriteButton(0, 1, 64, 64, { 70.0f, size.y - 70.0f });
		clearButton.LoadSprite(AssetHolder::Get().GetTexture("buttons"));

		text.setFont(AssetHolder::Get().GetFont("lucidaConsole"));
		text.setCharacterSize(25);

		music.openFromFile("files/sounds/gameBg.wav");
		music.setLoop(true);
		if (isMusicPlaying) music.play();
	}

	void Input() override {}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		
		pauseUI.Logic(e);
		if (!pauseUI.GetIsPaused()) textWindow.ManageEvent(e);

		if (!isEditorRunState) {
			switch (e.type) {
			case sf::Event::KeyPressed:
				if (isHowToPlay) {
					background.setTexture(AssetHolder::Get().GetTexture("background"));
					isHowToPlay = false;
				}
				switch (e.key.code) {
				case sf::Keyboard::Q:
					if (pauseUI.GetIsPaused()) {
						SetState(State::Menu);
					}
					break;
				case sf::Keyboard::LControl:
					isKeyPressed = true;
					break;
				case sf::Keyboard::R:
					if (isKeyPressed) player.Run(levelManager.GetLevel(), isRun, textWindow.GetStrings());
					break;
				}
				break;
			case sf::Event::KeyReleased:
				switch (e.key.code) {
				case sf::Keyboard::LControl:
					isKeyPressed = false;
					break;
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (isHowToPlay) {
					background.setTexture(AssetHolder::Get().GetTexture("background"));
					isHowToPlay = false;
				}

				switch (e.key.code) {
				case sf::Mouse::Left:
					if (pauseUI.GetIsPaused() && pauseUI.GetIsMusicTogglerPressed((float)e.mouseButton.x, (float)e.mouseButton.y)) {
						if (!isMusicPlaying) {
							music.pause();
						}
						else {
							music.play();
						}
					}
					break;
				}

				break;
			}
		}

		runButton.Logic(0, 0, e, mousePos);
		clearButton.Logic(0, 1, e, mousePos);
	}

	void Logic(float dt) override {

		if (transitionScreen.GetTransition()) {

			transitionScreen.Logic();

			if (!transitionScreen.isLoadNextLevel) {
				if (isEditorRunState) {
					SetState(State::Editor);
					isEditorRunState = false;
				}
				else {

					int index = levelManager.GetIndex();
					switch (index) {
					case 9:
						SetState(State::LevelScene);
						scenesType = 1;
						break;
					case 18:
						SetState(State::LevelScene);
						scenesType = 2;
						break;
					case 19:
						SetState(State::LevelScene);
						scenesType = 3;
						break;
					}

					levelManager.LoadNextLevel();
					textManager.LoadNextText();
					textBox.setSize({ (float)windowSize.x, (float)textManager.GetTextSize() * 16.0f });
					textWindow.ResetStrings();
					Initialize();
				}

				transitionScreen.isLoadNextLevel = true;
			}
		}

		if (pauseUI.GetIsPaused()) return;

		textWindow.Logic();

		if (runButton.GetIsPressed() && isButtonPressable) {
			isRun = true;
			isButtonPressable = false;

			player.Run(levelManager.GetLevel(), isRun, textWindow.GetStrings());
		}

		if (clearButton.GetIsPressed()) textWindow.ResetStrings();

		int time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		t += time;

		if (isRun) {
			if (t > delay) {

				if (player.GetMovePositions().size() > 0) {

					sf::Vector2f playerDirection = player.GetCurrentMovePosition();
					player.Move(boxes, opponent, levelManager.GetLevel());
					sf::Vector2f playerPos = player.GetPosition();

					for (auto& box : boxes)
						box.Logic(levelManager.GetLevel(), playerPos, playerDirection);

					for (auto& tile : tiles)
						tile.Logic(boxes);

					opponent.Move(player.GetPosition());
				}

				t = 0;
			}
		}

		player.Logic(levelManager.GetItemMap(), isRun, !isToggleTileInLevel, tiles);

		if (player.GetIsWin() && t > 2 * delay) {
			transitionScreen.SetTransition(true);
		}

		if (t > 2 * delay && !player.GetIsWin()) {
			for (auto& box : boxes) box.Reset();
			for (auto& tile : tiles) tile.Reset();

			for (const auto& a : player.GetChangedTiles()) {
				levelManager.GetLevel().SetCharacter(a.x, a.y, '.');
			}

			player.Reset();
			if (isOpponentInLevel) opponent.Reset();
			isButtonPressable = true;
			t = 0;
		}
	}

	void SetRect(int x, int y) {
		spriteTile.setTextureRect(sf::IntRect(x * (int)pixelSize, y * (int)pixelSize, (int)pixelSize, (int)pixelSize));
	}

	void Render(sf::RenderWindow& window) override {

		//Background
		window.draw(background);

		if (isHowToPlay && !isEditorRunState) return;

		for (int i = 1; i < (int)levelManager.GetLevel().GetHeight() - 1; i++) {
			for (int j = 1; j < (int)levelManager.GetLevel().GetWidth() - 1; j++) {
				switch (levelManager.GetLevel().GetCharacter(j, i)) {
				case '#':
					SetRect(1, 1);
					break;
				case '1':
					SetRect(0, 0);
					break;
				case '2':
					SetRect(1, 0);
					break;
				case '3':
					SetRect(2, 0);
					break;
				case '4':
					SetRect(2, 1);
					break;
				case '5':
					SetRect(2, 2);
					break;
				case '6':
					SetRect(1, 2);
					break;
				case '7':
					SetRect(0, 2);
					break;
				case '8':
					SetRect(0, 1);
					break;
				case '9':
					SetRect(3, 0);
					break;
				case '.':
					continue;
					break;
				}

				spriteTile.setPosition(j * pixelSize, i * pixelSize);
				window.draw(spriteTile);
			}
		}

		for (int i = 1; i < (int)levelManager.GetItemMap().GetHeight() - 1; i++) {
			for (int j = 1; j < (int)levelManager.GetItemMap().GetWidth() - 1; j++) {
				switch (levelManager.GetItemMap().GetCharacter(j, i)) {
				case '#':
				case '.':
					continue;
					break;
				case 'A':
					SetRect(5, 0);
					break;
				case 'S':
					SetRect(5, 1);
					break;
				case 'W':
					bool isWinTileActive = !isToggleTileInLevel;
					if (tiles.size() > 0) isWinTileActive = true;
					for (auto& tile : tiles) {
						if (!tile.GetIsTileActive()) {
							isWinTileActive = false;
							break;
						}
					}
					SetRect(4, isWinTileActive ? 0 : 1);
					break;
				}

				spriteTile.setPosition(j * pixelSize, i * pixelSize);
				window.draw(spriteTile);
			}
		}

		//Player
		player.Render(window);

		//Opponent
		opponent.Render(window);

		//Toggle Tile
		for (auto& tile : tiles) {
			spriteTile.setPosition(tile.GetPosition());
			SetRect(4, 2);
			window.draw(spriteTile);
		}

		//Boxes
		for (auto& box : boxes) {
			
			if (box.GetIsValue()) {

				bool isActive = false;
				for (auto& tile : tiles) {
					if (tile.GetPosition() == box.GetPosition()) {
						isActive = true;
						break;
					}
				}

				SetRect(3, isActive ? 2 : 1);
				spriteTile.setPosition(box.GetPosition());
				window.draw(spriteTile);
			}
		}

		//Buttons
		runButton.Render(window);
		clearButton.Render(window);

		//Text
		window.draw(textBox);
		if (!isEditorRunState) {
			textManager.DrawTextHelp({ 0.0f, 0.0f, }, window);
		}

		//TextWindow
		textWindow.Render(window);
		DrawLine(window, windowSize.x - 150.0f, 30.0f, (float)windowSize.x, 30.0f);
		text.setString("Console");
		text.setPosition(windowSize.x - 130.0f, 0.0f);
		window.draw(text);

		DrawTextWithValue(window, AssetHolder::Get().GetFont("lucidaConsole"), 160.0f, (windowSize.y - 32.0f), "Moves :", player.GetNMoves(), sf::Color::White, 25);

		if (transitionScreen.GetTransition()) {
			transitionScreen.Render(window);
		}
		else {
			pauseUI.Render(window);
		}
	}
};

class Game {
private:
	sf::RenderWindow Window;
	const sf::String windowTitle;
	sf::Vector2u windowSize;

	std::unique_ptr<GameState> gameState;

	sf::Clock clock;
	float initDt;
	bool showFPS;

	void LoadAssets() {
		AssetHolder::Get().AddFont("lucidaConsole", "files/fonts/Lucida_Console.ttf");
		AssetHolder::Get().AddTexture("Tileset", "files/images/Tileset.png");
		AssetHolder::Get().AddTexture("buttons", "files/images/buttons.png");
		AssetHolder::Get().AddTexture("menuButtons", "files/images/menuButtons.png");
		AssetHolder::Get().AddTexture("player", "files/images/player.png");
		AssetHolder::Get().AddTexture("howToPlay", "files/images/howToPlay.png");
		AssetHolder::Get().AddTexture("background", "files/images/gameBack.png");
		AssetHolder::Get().AddTexture("menuBackground", "files/images/menuBack.png");
		AssetHolder::Get().AddTexture("soundToggler", "files/images/soundToggle.png");

		//Game scenes
		AssetHolder::Get().AddTexture("scene01", "files/images/scenes/scene01.png");
		AssetHolder::Get().AddTexture("scene02", "files/images/scenes/scene02.png");
		AssetHolder::Get().AddTexture("scene03", "files/images/scenes/scene03.png");
		AssetHolder::Get().AddTexture("scene04", "files/images/scenes/scene04.png");
		AssetHolder::Get().AddTexture("scene05", "files/images/scenes/scene05.png");
		AssetHolder::Get().AddTexture("scene06", "files/images/scenes/scene06.png");
		AssetHolder::Get().AddTexture("scene07", "files/images/scenes/scene07.png");
		AssetHolder::Get().AddTexture("scene08", "files/images/scenes/scene08.png");
		AssetHolder::Get().AddTexture("scene09", "files/images/scenes/scene09.png");
		AssetHolder::Get().AddTexture("scene11", "files/images/scenes/scene11.png");
		AssetHolder::Get().AddTexture("scene12", "files/images/scenes/scene12.png");
		AssetHolder::Get().AddTexture("scene21", "files/images/scenes/scene21.png");
		AssetHolder::Get().AddTexture("scene22", "files/images/scenes/scene22.png");
		AssetHolder::Get().AddTexture("scene31", "files/images/scenes/scene31.png");
		AssetHolder::Get().AddTexture("scene32", "files/images/scenes/scene32.png");
		AssetHolder::Get().AddTexture("scene33", "files/images/scenes/scene33.png");
		AssetHolder::Get().AddTexture("scene34", "files/images/scenes/scene34.png");
	}

	template<typename T>
	void SetState() {
		gameState = std::make_unique<T>(Window.getSize());
		gameState->isStateChanged = false;
	}
public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: windowSize(x, y),
		windowTitle(title),
		Window({ x, y }, title, sf::Style::Titlebar | sf::Style::Close) {
		Window.setFramerateLimit(60);

		LoadAssets();
		SetState<MenuState>();

		initDt = 0.0f;
		showFPS = false;
	}

	void Logic() {
		while (Window.isOpen()) {
			sf::Event e;

			float currentDt = (float)clock.getElapsedTime().asSeconds();
			float frameDt = currentDt - initDt;
			initDt = currentDt;

			sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(Window);

			if (gameState->isStateChanged) {
				if (gameState->state == GameState::State::Play) {
					SetState<PlayState>();
				}
				else if (gameState->state == GameState::State::LevelScene) {
					SetState<SceneState>();
				}
				else if (gameState->state == GameState::State::Editor) {
					SetState<EditorState>();
				}
				else if (gameState->state == GameState::State::Menu) {
					SetState<MenuState>();
				}
				else if (gameState->state == GameState::State::Quit) {
					Window.close();
					break;
				}
			}

			auto& currentGameState = gameState;

			while (Window.pollEvent(e)) {
				switch (e.type) {
				case sf::Event::Closed:
					Window.close();
					gameState->MusicClear();
					break;
				case sf::Event::KeyPressed:
					switch (e.key.code) {
					case sf::Keyboard::F3:
						showFPS = !showFPS;
						break;
					}
					break;
				}

				currentGameState->ManageEvent(e, mousePos);
			}

			currentGameState->Input();
			currentGameState->Logic(frameDt);

			Window.clear();
			currentGameState->Render(Window);
			if (showFPS) {
				DrawTextWithValue(Window, AssetHolder::Get().GetFont("lucidaConsole"), 0.0f, 0.0f, "FPS :", (int)(1.0f / frameDt));
			}
			Window.display();
		}
	}

	void Run() {
		Logic();
	}
};

int main() {

	srand((unsigned)time(0));

	Game game(512, 512, "Code Adventures");
	game.Run();

	return 0;
}