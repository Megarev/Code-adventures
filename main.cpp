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
				if (stringText[0] == "rotate") {
					colors[stringIndex] = sf::Color::Yellow;
					if (i == textIndex) {
						std::string activeString = textString.str();

						while (activeString.size() > (uint32_t)(i == 0 ? 9 : 10)) {
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

						colors[j] = sf::Color(255, 100, 0);

						if (loopText[0] == "move") {
							colors[j] = sf::Color(0, 200, 100);
						}

						if (loopText[0] == "rotate") {
							colors[j] = sf::Color(200, 100, 0);
						}

						if (loopText[0] == "if") colors[j] = sf::Color(100, 255, 255);

						if (loopText[0] == "else" || loopText[0] == "closeif" || loopText[0] == "endif") colors[j] = sf::Color(100, 255, 255);

						if (loopText[0] == "closeloop" || loopText[0] == "end") { //Openloop close
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

				if (strView[index] == 'r') {
					if (strView.size() < 9) {
						if (strView[index + 1] == 'e') {
							text.setString(sf::String("> repeat n\n> statements\n> end"));
							window.draw(text);
						}
						else {
							text.setString(sf::String("> rotate [dir]"));
							window.draw(text);
						}
					}
				}

				if (strView[index] == 'i') {
					if (strView.size() < 12) {
						text.setString(sf::String("> if fd empty\n> then []\n> else\n> else []\n> endif"));
						window.draw(text);
					}
				}
			}
			
			text.setFillColor(colors[i]);
			text.setString("> " + strings[i] + (i == textIndex ? "_" : ""));
			window.draw(text);

			if (stringText.size() > 1) {
				if (stringText[0] == "move" || stringText[0] == "openloop" || stringText[0] == "repeat" || stringText[0] == "rotate") {
					if (stringText[0] == "move") text.setFillColor(sf::Color::Green);
					else if (stringText[0] == "openloop" || stringText[0] == "repeat") text.setFillColor(sf::Color::Magenta);
					else if (stringText[0] == "rotate") text.setFillColor(sf::Color(200, 100, 0));
					text.setString(stringText[1]);

					float positionX = (float)window.getSize().x - stringText[0].size() * text.getCharacterSize();
					if (stringText[0] == "move") { positionX -= 30.0f; }
					else if (stringText[0] == "openloop") { positionX += 67.0f; }
					else if (stringText[0] == "repeat" || stringText[0] == "rotate") { positionX += 18.0f; }

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

bool isEditorRunState = false;

class GameState {
public:
	sf::Vector2u windowSize;
	bool isStateChanged;
	enum class State {
		Menu = 0,
		Play = 1,
		Editor = 2,
		Quit = 3
	} state;
	
	sf::Music music; //Background music

	GameState(const sf::Vector2u& size)
		: windowSize(size) {

		state = State::Menu;
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
	}
};

class MenuState : public GameState {
private:
	gui::SpriteButton playButton, editorButton, quitButton;
public:
	MenuState(const sf::Vector2u& size)
		: GameState(size) {
		playButton = gui::SpriteButton(0, 0, 200, 64, { 150.0f, 192.0f });
		playButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		editorButton = gui::SpriteButton(0, 2, 200, 64, { 150.0f, 272.0f });
		editorButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		quitButton = gui::SpriteButton(0, 1, 200, 64, { 150.0f, 352.0f });
		quitButton.LoadSprite(AssetHolder::Get().GetTexture("menuButtons"));

		music.openFromFile("files/sounds/menuBg.wav");
		music.setLoop(true);
		music.play();
	}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		playButton.Logic(0, 0, e, mousePos);
		quitButton.Logic(0, 1, e, mousePos);

		editorButton.Logic(0, 2, e, mousePos);
	}

	void Logic(float dt) override {
		if (playButton.GetIsPressed()) {
			SetState(State::Play);
			music.stop();
		}
		else if (quitButton.GetIsPressed()) {
			SetState(State::Quit);
		}
		else if (editorButton.GetIsPressed()) {
			SetState(State::Editor);
		}
	}

	void Render(sf::RenderWindow& window) override {
		playButton.Render(window);
		quitButton.Render(window);
		editorButton.Render(window);
	}
};

class Box {
private:
	sf::Vector2f resetPosition, position;
public:
	Box() {}
	Box(const sf::Vector2f& pos)
		: resetPosition(pos), position(pos) {}

	void Logic(Level& level, const sf::Vector2f& playerPos, const sf::Vector2f& direction) {

		auto [x, y] = (sf::Vector2i)(position / pixelSize);

		if (position == playerPos && level.GetCharacter(x + (int)direction.x, y + (int)direction.y) == '#') {
			position += direction * pixelSize;
		}
	}

	void Reset() { position = resetPosition; }
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

class Player {
private:
	sf::Vector2f resetPlayerPos, playerPos;
	sf::Sprite sprite;
	std::vector<sf::Vector2i> movePositions;
	std::vector<int> rotations;
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
		} else if (text == "bk") { //Move backward
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
					IntrepretMovement(text[1]);
				}
				else if (text[0] == "if") {
					int nLines = (int)IntrepretCheckConditional(level, strings, conditionalPoint);
					value += nLines;
				}
				else if (text[0] == "rotate") {
					IntrepretRotation(text[1]);
					rotations.push_back(direction);
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
					for (const sf::Vector2i& pos : movePositions) {
						x += pos.x;
						y += pos.y;
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
			else if (text[0] == "rotate") IntrepretRotation(text[1]);
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

	void Move(const std::vector<Box>& boxes, const Level& level) {
		//Player coords in level
		auto [x, y] = (sf::Vector2i)(playerPos / pixelSize);

		if (movePositions.size() > 0) {

			bool isMove = true;

			//New Position of player in level
			sf::Vector2i playerToLevelIndex = { x + movePositions[index].x, y + movePositions[index].y };
			sf::Vector2f newPlayerPos = (sf::Vector2f)playerToLevelIndex * pixelSize;

			sprite.setTextureRect(sf::IntRect(rotations[index] * (int)pixelSize, 0, (int)pixelSize, (int)pixelSize));

			for (auto& box : boxes) {
				if (newPlayerPos == box.GetPosition() && level.GetCharacter(playerToLevelIndex.x + movePositions[index].x, 
					playerToLevelIndex.y + movePositions[index].y) != '#') {
					isMove = false;
				}
			}

			if (isMove && level.GetCharacter(playerToLevelIndex.x, playerToLevelIndex.y) == '#') {
				nMoves++;
				playerPos += sf::Vector2f(movePositions[index].x * pixelSize, movePositions[index].y * pixelSize);
			}
			
			index++;	
		}
	}

	void Run(const Level& level, bool& isRun, const std::vector<std::string>& strings) {
		movePositions.clear();
		rotations.clear();

		int value = 0;
		for (int i = 0; i < (int)strings.size(); i++) {

			//StringIndex ensures that looping & conditional strings are not re-read
			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) break;

			int x = 0, y = 0;
			for (const sf::Vector2i& pos : movePositions) {
				x += pos.x;
				y += pos.y;
			}

			if (level.GetCharacter((uint32_t)x, (uint32_t)y) == 'W') {
				isRun = true;
				break;
			}

			auto text = ToWords(strings[stringIndex]);
			if (text.size() == 0) continue;

			if (text[0] == "move") {
				IntrepretMovement(text[1]);
			}

			if (text[0] == "rotate") {
				IntrepretRotation(text[1]);
				rotations.push_back(direction);
			}

			if (text[0] == "if") {
				int nLines = (int)IntrepretCheckConditional(level, strings, stringIndex) - 1;
				value += nLines;
			}

			if (text[0] == "openloop" || text[0] == "repeat") {

				int n = std::stoi(text[1]);

				//nLines -> number of lines in loop
				int nLines = (int)IntrepretLoop(level, isRun, strings, stringIndex, n);
				value += nLines;
			}
		}
	}

	void Logic(Level& itemMap, bool& isRun, bool isWinTileActive, ToggleTile& tile) {
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
		case 'W':
			if ((!isWinTileActive && tile.GetIsTileActive()) || (isWinTileActive)) isWin = true;
			break;
		case 'S':
			movePositions.clear();
			Reset();
			break;
		}

		if (index > (int)(movePositions.size() - 1)) {
			isRun = false;
			isIndex = true;
		}
	
		sprite.setPosition(playerPos);
	}

	void AddMovePosition(int dirX, int dirY) {
		rotations.push_back(direction);
		movePositions.emplace_back(dirX, dirY);
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
	
	std::vector<sf::Vector2i> GetMovePositions() const { return movePositions; }
	sf::Vector2f GetCurrentMovePosition() { 
		int posIndex = index;
		if (posIndex > (int)(movePositions.size() - 1)) {
			posIndex = (int)(movePositions.size() - 1);
		}
		return (sf::Vector2f)movePositions[posIndex];
	}
};

typedef std::vector<std::vector<std::string>> Texts;
class TextManager {
private:
	Texts texts;
	int index;

	void LoadTexts(const std::string& filepath) {
		std::ifstream reader(filepath);

		if (reader.is_open()) {

			while (!reader.eof()) {

				std::vector<std::string> initTexts;

				while (true) {
					std::string str;
					//reader >> readStr;
					std::getline(reader, str);
					if (str == ">") break;
					
					initTexts.push_back(str);
				}

				texts.push_back(initTexts);
			}

			reader.close();
		}
	}
public:
	TextManager() {
		LoadTexts("files/text.txt");
		index = 0;
	}

	void LoadNextText() {
		++index %= (int)texts.size();
	}

	Texts GetTexts() const { return texts; }

	void DrawTextHelp(const sf::Vector2f& pos, sf::RenderWindow& window) {

		float textPos = pos.y;

		for (int i = 0; i < (int)texts[index].size(); i++) {
			RenderText(window, AssetHolder::Get().GetFont("lucidaConsole"), pos.x, pos.y + i * 16.0f, texts[index][i], sf::Color::White, 16);
		}
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

	void ResetIndex() { index = 0; }
	void LoadNextLevel() { 
		++index %= (int)strings.size();
		LoadLevel();
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

class PlayState : public GameState {
private:
	LevelManager levelManager; //Level Manager
	TextManager textManager; //Text Manager

	Transition transitionScreen; //Transition between levels

	sf::Sprite spriteTile, background;

	Player player;

	std::vector<Box> boxes;
	ToggleTile tile;

	TextWindow textWindow;
	gui::SpriteButton runButton, clearButton;
	sf::Text text;

	bool isRun, isButtonPressable, isHowToPlay, isToggleTileInLevel;

	sf::Clock clock;
	int t, delay;

	void Initialize() {

		boxes.clear();
		player.Reset();
		tile.SetPosition({ -pixelSize, -pixelSize });

		ItemMap& map = levelManager.GetItemMap();

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
				case 'T': //ToggleTile Position
					tile = ToggleTile({ j * pixelSize, i * pixelSize });
					map.SetCharacter(j, i, '#');
					isToggleTileInLevel = true;
					break;
				}
			}
		}
	}
public:
	PlayState(const sf::Vector2u& size) 
		: GameState(size) {
		
		textWindow = TextWindow({ (float)size.x - 150.0f, 0.0f }, { 150.0f, (float)size.y });
		textWindow.SetFont(AssetHolder::Get().GetFont("lucidaConsole"));

		spriteTile.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		player.LoadSprite(AssetHolder::Get().GetTexture("player"));

		transitionScreen = Transition((sf::Vector2f)size);

		if (isEditorRunState) {
			background.setTexture(AssetHolder::Get().GetTexture("background"));
			levelManager.LoadLevelFromFile("files/levels/EditorLevel.lvl");
			levelManager.LoadItemMap("files/levels/EditorLevelItemMap.lvl");
		}
		else {
			background.setTexture(AssetHolder::Get().GetTexture("howToPlay"));
		}

		Initialize();

		delay = 100; //Milliseconds

		isRun = true;
		player.Run(levelManager.GetLevel(), isRun, textWindow.GetStrings());

		isButtonPressable = true;
		isHowToPlay = true; //Is How To Play background rendered

		runButton = gui::SpriteButton(0, 0, 64, 64, { 0.0f, size.y - 70.0f });
		runButton.LoadSprite(AssetHolder::Get().GetTexture("buttons"));
		clearButton = gui::SpriteButton(0, 1, 64, 64, { 70.0f, size.y - 70.0f });
		clearButton.LoadSprite(AssetHolder::Get().GetTexture("buttons"));

		text.setFont(AssetHolder::Get().GetFont("lucidaConsole"));
		text.setCharacterSize(25);

		music.openFromFile("files/sounds/gameBg.wav");
		music.setLoop(true);
		music.play();
	}

	void Input() override {
		if (KeyPress(sf::Keyboard::Escape)) {
			SetState(isEditorRunState ? State::Editor : State::Menu);
			music.stop();
		}
	}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		textWindow.ManageEvent(e);
		
		if (!isEditorRunState) {
			switch (e.type) {
			case sf::Event::KeyPressed:
				if (isHowToPlay) {
					background.setTexture(AssetHolder::Get().GetTexture("background"));
					isHowToPlay = false;
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (isHowToPlay) {
					background.setTexture(AssetHolder::Get().GetTexture("background"));
					isHowToPlay = false;
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
					levelManager.LoadNextLevel();
					textManager.LoadNextText();
					textWindow.ResetStrings();
					Initialize();
				}
			
				transitionScreen.isLoadNextLevel = true;
			}
		}

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
					player.Move(boxes, levelManager.GetLevel());
					sf::Vector2f playerPos = player.GetPosition();

					for (auto& box : boxes)
						box.Logic(levelManager.GetLevel(), playerPos, playerDirection);

					tile.Logic(boxes);
				}

				t = 0;
			}
		}

		player.Logic(levelManager.GetItemMap(), isRun, !isToggleTileInLevel, tile);

		if (player.GetIsWin() && t > 2 * delay) {
			transitionScreen.SetTransition(true);
		}

		if (t > 2 * delay && player.GetIsIndex()) {
			for (auto& box : boxes) box.Reset();
			tile.Reset();
			if (!player.GetIsWin()) player.Reset();
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
				case 'T':
					SetRect(4, 2);
					break;
				case 'W':
					bool isWinTileActive = !isToggleTileInLevel;
					if (tile.GetIsTileActive()) isWinTileActive = true;
					SetRect(4, isWinTileActive ? 0 : 1);
					break;
				}

				spriteTile.setPosition(j * pixelSize, i * pixelSize);
				window.draw(spriteTile);
			}
		}

		//Player
		player.Render(window);

		//Boxes
		for (auto& box : boxes) {
			SetRect(3, tile.GetIsTileActive() ? 2 : 1);
			spriteTile.setPosition(box.GetPosition());
			window.draw(spriteTile);
		}

		//TextWindow
		textWindow.Render(window);
		DrawLine(window, windowSize.x - 150.0f, 30.0f, (float)windowSize.x, 30.0f);
		text.setString("Console");
		text.setPosition(windowSize.x - 130.0f, 0.0f);
		window.draw(text);

		//Buttons
		runButton.Render(window);
		clearButton.Render(window);

		//Text
		if (!isEditorRunState) {
			textManager.DrawTextHelp({ 0.0f, 0.0f, }, window);
		}

		DrawTextWithValue(window, AssetHolder::Get().GetFont("lucidaConsole"), 160.0f, (windowSize.y - 32.0f), "Moves :", player.GetNMoves(), sf::Color::White, 25);
	
		if (transitionScreen.GetTransition()) {
			transitionScreen.Render(window);
		}
	}
};

class Game {
private:
	sf::RenderWindow Window;
	const sf::String windowTitle;

	std::unique_ptr<GameState> gameState;
	
	sf::Clock clock;
	float initDt;
	bool showFPS, isFullScreen;

	void LoadAssets() {
		AssetHolder::Get().AddFont("lucidaConsole", "files/fonts/Lucida_Console.ttf");
		AssetHolder::Get().AddTexture("Tileset", "files/images/Tileset.png");
		AssetHolder::Get().AddTexture("buttons", "files/images/buttons.png");
		AssetHolder::Get().AddTexture("menuButtons", "files/images/menuButtons.png");
		AssetHolder::Get().AddTexture("player", "files/images/player.png");
		AssetHolder::Get().AddTexture("howToPlay", "files/images/howToPlay.png");
		AssetHolder::Get().AddTexture("background", "files/images/gameBack.png");
	}

	template<typename T>
	void SetState() {
		gameState = std::make_unique<T>(Window.getSize());
		gameState->isStateChanged = false;
	}

	void ToggleScreenMode() {
		isFullScreen = !isFullScreen;

		if (isFullScreen) {
			Window.create({ 512, 512 }, windowTitle, sf::Style::Fullscreen);
		}
		else {
			Window.create({ 512, 512 }, windowTitle, sf::Style::Default);
		}
	}
public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: windowTitle(title),
		  Window({ x, y }, title) {
		Window.setFramerateLimit(60);
	
		LoadAssets();
		SetState<MenuState>();

		initDt = 0.0f;
		isFullScreen = false;
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
					case sf::Keyboard::F2:
						ToggleScreenMode();
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