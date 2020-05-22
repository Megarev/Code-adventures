#include <SFML/Graphics.hpp>
#include "GraphicsRender.h"
#include "AssetManager.h"
#include "GraphicsUI.h"
#include <iterator>
#include <ctime>

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
	uint32_t nShowText, charSpacing;  //nShowText for rendering, charSpacing -> distance between 2 characters
	sf::Text text;					  //Text for rendering to the window

	void Input(int character) {

		if (character < 0) return;

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
public:
	TextWindow() {}
	TextWindow(const sf::Vector2f& pos, const sf::Vector2f& size) {
		box.setSize(size);
		box.setPosition(pos);
		box.setFillColor(sf::Color(25, 25, 25));
		text.setCharacterSize(15);

		colors.push_back(sf::Color::White);
		strings.push_back("");
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
			
			if (stringText[0] == "move") {
				colors[stringIndex] = sf::Color::Cyan;
			}
			else if (stringText[0] == "openloop" || stringText[0] == "repeat") {
				for (int j = stringIndex; j < (int)strings.size(); j++) {

					auto loopText = ToWords(strings[j]);
					if (loopText.size() == 0) continue;

					colors[j] = sf::Color(255, 100, 0);

					if (loopText[0] == "move") {
						colors[j] = sf::Color(0, 200, 100);
					}

					if (loopText[0] == "closeloop" || loopText[0] == "end") { //Openloop close
						colors[j] = sf::Color(255, 150, 0);
						break;
					}
					value++;
				}
			}
		}
	}

	void Render(sf::RenderWindow& window) {
		window.draw(box);

		float pos = 2.0f;

		for (int i = showTextIndex; i < (int)strings.size(); i++) {

			auto stringText = ToWords(strings[i]);

			text.setFillColor(colors[i]);
			text.setString("> " + strings[i] + (i == textIndex ? "_" : ""));
			text.setPosition(360.0f, pos * (float)text.getCharacterSize());
			window.draw(text);
			
			if (stringText.size() > 1) {
				if (stringText[0] == "move" || stringText[0] == "openloop" || stringText[0] == "repeat") {
					text.setFillColor(stringText[0] == "move" ? sf::Color::Green : sf::Color::Magenta);
					text.setString(stringText[1]);

					float positionX = stringText[0].size() * text.getCharacterSize() / 2.0f;
					if (stringText[0] == "move") { positionX += 32.0f; }
					if (stringText[0] == "openloop") { positionX += 40.0f; }
					if (stringText[0] == "repeat") { positionX += 35.0f; }

					text.setPosition(360.0f + positionX, pos * (float)text.getCharacterSize());
					window.draw(text);
				}
			}
			
			pos++;
		}
	}

	std::vector<sf::Color>& Colors() { return colors; }
	inline std::vector<std::string> GetStrings() const { return strings; }
};

class GameState {
public:
	sf::Vector2u windowSize;
	GameState(const sf::Vector2u& size)
		: windowSize(size) {}

	virtual void Input() {}
	virtual void ManageEvent(sf::Event, sf::Vector2f) {}
	virtual void Logic(float) = 0;
	virtual void Render(sf::RenderWindow&) = 0;

	static bool KeyPress(sf::Keyboard::Key key) {
		return sf::Keyboard::isKeyPressed(key);
	};
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

		if (position == playerPos && level.GetLevel()[y + (int)direction.y][x + (int)direction.x] == '#') {
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
	std::vector<sf::Vector2i> movePositions;
	int index, nMoves;
	bool isIndex, isWin; //isIndex -> index == (movePositions.size - 1) & isWin -> has player won a level
	sf::Color color;

	void IntrepretMovement(const std::vector<std::string>& text) {
		if (text[1] == "l") {
			AddMovePosition(-1, 0);
		}
		else if (text[1] == "r") {
			AddMovePosition(1, 0);
		}
		else if (text[1] == "u") {
			AddMovePosition(0, -1);
		}
		else if (text[1] == "d") {
			AddMovePosition(0, 1);
		}
	}

	std::size_t IntrepretLoop(const std::vector<std::string>& strings, int nStartPoint, int nLoop) {

		std::vector<std::string> stringsToLoop;
		bool isLoopPoint = false;

		for (int i = nStartPoint; i < (int)strings.size(); i++) {

			auto textForLoop = ToWords(strings[i]);
			if (textForLoop.size() == 0) continue;

			if (textForLoop[0] == "closeloop" || textForLoop[0] == "end") { //Openloop close
				isLoopPoint = true;
				break;
			}
			else if (textForLoop[0] == "openloop" || textForLoop[0] == "repeat") {
				continue;
			}

			stringsToLoop.push_back(strings[i]);
		}

		if (!isLoopPoint) return 0;

		for (int n = 0; n < nLoop; n++) {
			for (int i = 0; i < (int)stringsToLoop.size(); i++) {

				auto text = ToWords(stringsToLoop[i]);
				if (text.size() == 0) continue;

				if (text[0] == "move") {
					IntrepretMovement(text);
				}
			}
		}

		return stringsToLoop.size();
	}
public:
	Player() {
		index = 0;
		nMoves = 0;
		isIndex = false;
		isWin = false;

		color = sf::Color(rand() % 256, rand() % 256, rand() % 256);
	}

	void Move(const Level& level) {
		//Player coords in level
		auto [x, y] = (sf::Vector2i)(playerPos / pixelSize);

		if (movePositions.size() > 0) {

			//New Position of player in level
			sf::Vector2i playerToLevelIndex = { x + movePositions[index].x, y + movePositions[index].y };

			if (level.GetLevel()[playerToLevelIndex.y][playerToLevelIndex.x] == '#') {
				playerPos += sf::Vector2f(movePositions[index].x * pixelSize, movePositions[index].y * pixelSize);
			}
			
			index++;	
		}
	}

	void Run(const std::vector<std::string>& strings) {
		ClearMovePositions();

		int value = 0;
		for (int i = 0; i < (int)strings.size(); i++) {

			//StringIndex ensures that looping strings are not re-read
			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) break;

			auto text = ToWords(strings[stringIndex]);
			if (text.size() == 0) continue;

			if (text[0] == "Move" || text[0] == "move") {
				IntrepretMovement(text);
			}

			if (text[0] == "openloop" || text[0] == "repeat") {

				int n = std::stoi(text[1]);

				//nLines -> number of lines in loop
				int nLines = (int)IntrepretLoop(strings, stringIndex, n);
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
		case '1':
			itemMap.SetCharacter((unsigned)x, (unsigned)y, '#');
			break;
		case 'W':
			if ((!isWinTileActive && tile.GetIsTileActive()) || (isWinTileActive)) isWin = true;
			break;
		}

		if (index > (int)(movePositions.size() - 1)) {
			isRun = false;
			isIndex = true;
		}
	}

	void AddMovePosition(int dirX, int dirY) {
		movePositions.emplace_back(dirX, dirY);
	}

	void ClearMovePositions() {
		movePositions.clear();
	}

	void ResetNMoves() { nMoves = 0; }
	void ResetIndex() { index = 0; }
	void ResetWin() { isWin = false; }
	void SetPosition(const sf::Vector2f& pos) {
		playerPos = pos;
		resetPlayerPos = pos;
	}

	inline sf::Color GetColor() const { return color; }
	inline int GetNMoves() const { return nMoves; }
	inline int GetIndex() const { return index; }
	inline bool GetIsWin() const { return isWin; }
	bool& GetIsIndex() { return isIndex; }
	inline sf::Vector2f GetPosition() const { return playerPos; }
	inline sf::Vector2f GetResetPosition() const { return resetPlayerPos; }
	
	std::vector<sf::Vector2i> GetMovePositions() const { return movePositions; }
	sf::Vector2f GetCurrentMovePosition() { 
		int posIndex = index;
		if (posIndex > (int)(movePositions.size() - 1)) {
			posIndex = (int)(movePositions.size() - 1);
		}
		return (sf::Vector2f)movePositions[posIndex]; 
	}
};

typedef Level ItemMap;
class LevelManager {
private:

	struct LevelData {
		std::string level, itemMap;
		bool isTileActive; //Win Tile

		LevelData(const std::string& levelStr, const std::string& itemMapStr, bool isActive)
			: level(levelStr), itemMap(itemMapStr), isTileActive(isActive) {}
	};

	std::vector<LevelData> strings;
	std::string folder, extension;
	
	int index;

	Level level;
	ItemMap itemMap;
	bool isWinTileActive;

	void LoadLevel() {
		level = Level::LoadLevel(folder + strings[index].level + extension);
		itemMap = Level::LoadLevel(folder + strings[index].itemMap + extension);
		isWinTileActive = strings[index].isTileActive;
	}
public:
	LevelManager() {
		folder = "files/levels/";
		extension = ".txt";
		index = 0;
	}

	void LoadLevelStrings(const std::string& filename) {
		std::ifstream reader(folder + filename + extension);

		if (reader.is_open()) {

			while (!reader.eof()) {
				std::string levelName, itemMapName;
				bool tileActive;
				reader >> levelName >> itemMapName >> tileActive;
				strings.emplace_back(levelName, itemMapName, tileActive);
			}

			reader.close();
		}

		if ((int)strings.size() > 0) {
			LoadLevel();
		}
	}

	Level& GetLevel() { return level; }
	ItemMap& GetItemMap() { return itemMap; }
	inline int GetIndex() const { return index; }
	inline bool GetIsWinTileActive() const { return isWinTileActive; }

	void ResetIndex() { index = 0; }
	void LoadNextLevel() { 
		++index %= (int)strings.size();
		LoadLevel();
	}
};

class PlayState : public GameState {
private:
	LevelManager levelManager; //Level Manager

	sf::RectangleShape pixel;
	sf::Sprite spriteTile, itemTile;

	Player player;

	std::vector<Box> boxes;
	ToggleTile tile;

	TextWindow textWindow;
	gui::SpriteButton runButton, clearButton;
	sf::Text text;

	bool isRun;

	sf::Clock clock;
	int t, delay;

	void Initialize() {

		boxes.clear();
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

		levelManager.LoadLevelStrings("LevelStrings");

		Initialize();

		spriteTile.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		itemTile.setTexture(AssetHolder::Get().GetTexture("coin"));

		pixel.setSize({ pixelSize, pixelSize });

		delay = 100; //Milliseconds

		isRun = false;

		runButton = gui::SpriteButton(0, 0, 64, { 0.0f, size.y - 70.0f });
		clearButton = gui::SpriteButton(0, 1, 64, { 70.0f, size.y - 70.0f });

		text.setFont(AssetHolder::Get().GetFont("lucidaConsole"));
		text.setCharacterSize(25);
	}

	void Input() override {}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		textWindow.ManageEvent(e);

		runButton.Logic(0, 0, e, mousePos);
		clearButton.Logic(0, 1, e, mousePos);
	}

	void Logic(float dt) override {

		textWindow.Logic();

		if (runButton.GetIsPressed()) {
			isRun = true;
			player.ResetIndex();
			
			player.Run(textWindow.GetStrings());
		}

		if (clearButton.GetIsPressed()) textWindow.ResetStrings();

		int time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		t += time;

		if (isRun) {
			if (t > delay) {
				
				if (player.GetMovePositions().size() > 0) {

					sf::Vector2f playerDirection = player.GetCurrentMovePosition();
					player.Move(levelManager.GetLevel());
					sf::Vector2f playerPos = player.GetPosition();

					for (auto& box : boxes)
						box.Logic(levelManager.GetLevel(), playerPos, playerDirection);

					tile.Logic(boxes);
				}

				t = 0;
			}
		}

		player.Logic(levelManager.GetItemMap(), isRun, levelManager.GetIsWinTileActive(), tile);

		if (player.GetIsWin() && t > 2 * delay) {
			levelManager.LoadNextLevel();
			textWindow.ResetStrings();
			Initialize();
			player.ResetWin();
		}

		if (t > 2 * delay && player.GetIsIndex()) {
			player.GetIsIndex() = false;
			player.ResetNMoves();
			for (auto& box : boxes) box.Reset();
			tile.Reset();
			player.SetPosition(player.GetResetPosition());
			t = 0;
		}
	}

	void SetRect(int x, int y) {
		spriteTile.setTextureRect(sf::IntRect(x * (int)pixelSize, y * (int)pixelSize, (int)pixelSize, (int)pixelSize));
	}

	void Render(sf::RenderWindow& window) override {
		for (int i = 1; i < (int)levelManager.GetLevel().GetHeight() - 1; i++) {
			for (int j = 1; j < (int)levelManager.GetLevel().GetWidth() - 1; j++) {
				switch (levelManager.GetLevel().GetLevel()[i][j]) {
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
				switch (levelManager.GetItemMap().GetLevel()[i][j]) {
				case '#':
				case '.':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
					continue;
					break;
				case 'A':
					itemTile.setPosition(j * pixelSize, i * pixelSize);
					window.draw(itemTile);
					break;
				case 'W':
					pixel.setFillColor(levelManager.GetIsWinTileActive() ? sf::Color::Green : sf::Color::Red);
					pixel.setPosition(j * pixelSize, i * pixelSize);
					window.draw(pixel);
					break;
				}
			}
		}
		
		//Player
		pixel.setPosition(player.GetPosition());
		pixel.setFillColor(player.GetColor());
		window.draw(pixel);

		//Toggle tile
		pixel.setPosition(tile.GetPosition());
		pixel.setFillColor(sf::Color::Magenta);
		window.draw(pixel);

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
		DrawTextWithValue(window, AssetHolder::Get().GetFont("lucidaConsole"), 160.0f, (windowSize.y - 32.0f), "Moves :", player.GetNMoves(), sf::Color::White, 25);
	}
};

class Game {
private:
	sf::RenderWindow Window;
	const sf::String windowTitle;

	std::vector<std::unique_ptr<GameState>> gameStates;
	
	sf::Clock clock;
	float initDt;
	bool showFPS;

	void LoadAssets() {
		AssetHolder::Get().AddFont("lucidaConsole", "files/fonts/Lucida_Console.ttf");
		AssetHolder::Get().AddTexture("Tileset", "files/images/Tileset.png");
		AssetHolder::Get().AddTexture("coin", "files/images/coin.png");
		AssetHolder::Get().AddTexture("buttons", "files/images/buttons.png");
	}

public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: windowTitle(title),
		  Window({ x, y }, title) {
		Window.setFramerateLimit(60);
	
		LoadAssets();
		gameStates.push_back(std::make_unique<PlayState>(sf::Vector2u(x, y)));
	
		initDt = 0.0f;
		showFPS = false;
	}

	void Logic() {
		while (Window.isOpen()) {
			sf::Event e;

			float currentDt = (float)clock.getElapsedTime().asSeconds();
			float frameDt = currentDt - initDt;
			initDt = currentDt;

			auto& currentGameState = gameStates.back();

			sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(Window);

			while (Window.pollEvent(e)) {
				switch (e.type) {
				case sf::Event::Closed:
					Window.close();
					break;
				case sf::Event::KeyPressed:
					switch (e.key.code) {
					case sf::Keyboard::LShift:
						showFPS = !showFPS;
						break;
					}
					break;
				case sf::Event::Resized:
					Window.create({ (uint32_t)e.size.width, (uint32_t)e.size.height }, windowTitle);
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

	Game game(512, 512, "Title");
	game.Run();

	return 0;
}