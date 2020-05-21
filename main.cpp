#include <SFML/Graphics.hpp>
#include "GraphicsRender.h"
#include "AssetManager.h"
#include "GraphicsUI.h"
#include <deque>
#include <regex>
#include <ctime>

const float pixelSize = 32.0f;

std::deque<std::string> RegexDeque(const std::regex& rgx, const std::string& initString) {
	std::deque<std::string> text(
		std::sregex_token_iterator(initString.begin(), initString.end(), rgx, -1),
		std::sregex_token_iterator()
	);

	return text;
}

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

			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) stringIndex = ((int)strings.size() - 1);
			
			std::string initString = strings[stringIndex];
			if (initString.size() > 1) {
				while (initString.back() == ' ' && initString.size() > 0) initString.pop_back();
			}
			
			auto stringText = RegexDeque(std::regex("\\s"), initString);

			colors[stringIndex] = sf::Color::White;

			if (stringText.size() > 2) stringText.pop_front();
			
			if (stringText[0] == "move") {
				colors[stringIndex] = sf::Color::Cyan;
			}
			else if (stringText[0] == "startloop" || stringText[0] == "repeat") {

				for (int j = stringIndex; j < (int)strings.size(); j++) {
					initString = strings[j];
					if (initString.size() > 1) {
						while (initString.back() == ' ' && initString.size() > 0) initString.pop_back();
					}

					auto loopText = RegexDeque(std::regex("\\s"), initString);
					if (loopText.size() > 2) loopText.pop_front();

					if (loopText.size() == 2) {
						if (loopText[0].size() == 0) loopText.pop_front();

						colors[j] = sf::Color(255, 100, 0);

						if (loopText[0] == "move") {
							colors[j] = sf::Color(0, 200, 100);
						}

						if (loopText[0] == "closeloop" || loopText[0] == "end") { //Startloop close
							break;
						}
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

			std::string initString = strings[i];

			auto stringText = RegexDeque(std::regex("\\s"), initString);
			if (stringText.size() > 2) stringText.pop_front();

			text.setFillColor(colors[i]);
			text.setString("> " + strings[i] + (i == textIndex ? "_" : ""));
			text.setPosition(360.0f, pos * (float)text.getCharacterSize());
			window.draw(text);
			
			if (stringText.size() > 1) {
				if (stringText[0] == "move" || stringText[0] == "startloop" || stringText[0] == "repeat") {
					text.setFillColor(stringText[0] == "move" ? sf::Color::Green : sf::Color::Magenta);
					text.setString(stringText[1]);

					float positionX = stringText[0].size() * text.getCharacterSize() / 2.0f;
					if (stringText[0] == "move") { positionX += 22.0f; }
					if (stringText[0] == "startloop") { positionX += 9.0f; }
					if (stringText[0] == "repeat") { positionX += 14.0f; }

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

class Player {
private:
	sf::Vector2f playerPos;
	std::vector<sf::Vector2i> movePositions;
	int index, nMoves;
	bool isIndex;

	void IntrepretMovement(const std::deque<std::string>& text, std::vector<sf::Color>& colors, bool isLoop) {
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

	std::size_t IntrepretLoop(const std::vector<std::string>& strings, std::vector<sf::Color>& colors, int nStartPoint, int nLoop) {

		std::vector<std::string> stringsToLoop;
		
		bool isLoopPoint = false;

		sf::Color randColor = sf::Color((rand() % 100) + 100, (rand() % 100) + 100, (rand() % 100) + 100);

		for (int i = nStartPoint; i < (int)strings.size(); i++) {

			std::string loopString = strings[i];
			while ((loopString.back() == '_' || loopString.back() == ' ') && loopString.size() > 0) loopString.pop_back();

			auto textForLoop = RegexDeque(std::regex("\\s"), loopString);
			//TextForLoop deque has an empty string as the first element when i > nStartPoint
			if (i > nStartPoint && textForLoop.size() > 0) textForLoop.pop_front();

			if (textForLoop.size() > 0) {
				if (textForLoop[0] == "closeloop" || textForLoop[0] == "end") { //Startloop close
					isLoopPoint = true;
					break;
				}
				else if (textForLoop[0] == "startloop" || textForLoop[0] == "repeat") {
					continue;
				}
			}

			stringsToLoop.push_back(loopString);
		}

		if (!isLoopPoint) return 0;

		for (int n = 0; n < nLoop; n++) {
			for (int i = 0; i < (int)stringsToLoop.size(); i++) {

				std::string initString = stringsToLoop[i];
				if (initString.back() == '_') initString.pop_back();

				auto text = RegexDeque(std::regex("\\s"), initString);
				if (text.size() > 2) text.pop_front();

				if (text[0] == "Move" || text[0] == "move") {
					IntrepretMovement(text, colors, true);
				}
			}
		}

		return stringsToLoop.size();
	}
public:
	Player() {}
	Player(const sf::Vector2f& pos)
		: playerPos(pos) {
		index = 0;
		nMoves = 0;
		isIndex = false;
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

	void Run(const std::vector<std::string>& strings, std::vector<sf::Color>& colors) {
		ClearMovePositions();

		int value = 0;

		system("cls");
		for (int i = 0; i < (int)strings.size(); i++) {

			//StringIndex ensures that looping strings are not re-read
			int stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) break;

			std::string initString = strings[stringIndex];
			if (initString.size() <= 1) continue;
			if (initString.size() > 1) {
				while ((initString.back() == '_' || initString.back() == ' ')) initString.pop_back();
			}

			auto text = RegexDeque(std::regex("\\s"), initString);

			//If i > 0 -> text.size() = 3, but text.size() == 2 is required
			if (text.size() > 2) text.pop_front();

			if (text[0] == "Move" || text[0] == "move") {
				IntrepretMovement(text, colors, false);
			}

			if (text[0] == "startloop" || text[0] == "StartLoop" || text[0] == "Repeat" || text[0] == "repeat") {

				std::istringstream ss(text[1]);
				int n = 0;
				ss >> n;

				//nLines -> number of lines in loop
				int nLines = (int)IntrepretLoop(strings, colors, stringIndex, n);
				value += nLines;
			}
		}
	}

	void Logic(Level& itemMap, int t, int delay, bool& isRun) {
		auto [x, y] = (sf::Vector2i)(playerPos / pixelSize);

		switch (itemMap.GetLevel()[y][x]) {
		case '1':
			itemMap.SetCharacter((unsigned)x, (unsigned)y, '#');
			break;
		case '2':
			std::cout << "Win" << std::endl;
			break;
		}

		if (index > (int)(movePositions.size() - 1)) {
			isRun = false;
			isIndex = true;
		}

		if (t > 2 * delay && isIndex) {
			isIndex = false;
			ResetNMoves();
			SetPosition({ pixelSize, pixelSize });
			t = 0;
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
	void SetPosition(const sf::Vector2f& pos) {
		playerPos = pos;
	}

	inline int GetNMoves() const { return nMoves; }
	inline int GetIndex() const { return index; }
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

class Box {
private:
	sf::Vector2f position;
public:
	Box() {}
	Box(const sf::Vector2f& pos)
		: position(pos) {}

	void Logic(Level& level, const sf::Vector2f& playerPos, const sf::Vector2f& direction) {
		
		auto [x, y] = (sf::Vector2i)(position / pixelSize);
		
		if (position == playerPos && level.GetLevel()[y + (int)direction.y][x + (int)direction.x] == '#') {
			position += direction * pixelSize;
		}
	}

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

	inline bool GetIsTileActive() const { return isTileActive; }
	inline sf::Vector2f GetPosition() const { return position; }
};

class PlayState : public GameState {
private:
	Level level, itemMap;
	sf::RectangleShape pixel;
	sf::Sprite spriteTile, itemTile;
	sf::CircleShape circle;

	Player player;
	sf::Color playerColor;

	std::vector<Box> boxes;
	ToggleTile tile;

	TextWindow textWindow;
	Button runButton, clearButton;
	sf::Text text;

	bool isRun, isGridDrawn;

	sf::Clock clock;
	int t, delay;

	void DrawGrid(sf::RenderWindow& window, float x, float y) {
		for (uint32_t i = 0; i < level.GetWidth() - 1; i++) {
			DrawLine(window, (x + i) * pixelSize, y * pixelSize, (x + i) * pixelSize, (float)(level.GetHeight() - 1) * pixelSize);
		}
		for (uint32_t i = 0; i < level.GetHeight() - 1; i++) {
			DrawLine(window, x * pixelSize, (y + i) * pixelSize, (float)(level.GetWidth() - 1) * pixelSize, (y + i) * pixelSize);
		}
	}
public:
	PlayState(const sf::Vector2u& size) 
		: GameState(size) {
		
		textWindow = TextWindow({ (float)size.x - 150.0f, 0.0f }, { 150.0f, (float)size.y });
		textWindow.SetFont(AssetHolder::Get().GetFont("sansationRegular"));

		player = Player({ pixelSize, pixelSize });
		playerColor = sf::Color(rand() % 256, rand() % 256, rand() % 256);

		boxes.push_back(Box({ 4 * pixelSize, 4 * pixelSize }));
		tile = ToggleTile({ 8 * pixelSize, 6 * pixelSize });

		spriteTile.setTexture(AssetHolder::Get().GetTexture("Tileset"));
		itemTile.setTexture(AssetHolder::Get().GetTexture("coin"));

		pixel.setSize({ pixelSize, pixelSize });
		circle.setRadius(10.0f);

		delay = 100; //Milliseconds

		isRun = false;
		isGridDrawn = true;

		runButton.Initialize({ 0.0f, size.y - 50.0f }, { 150.0f, 50.0f });
		runButton.SetColors(sf::Color(90, 100, 200), sf::Color(45, 50, 100), sf::Color(15, 25, 50));
		runButton.SetOutline(-2.0f, sf::Color(rand() % 256, rand() % 256, rand() % 256));

		clearButton.Initialize({ 155.0f, size.y - 50.0f }, { 50.0f, 50.0f });
		clearButton.SetColors(sf::Color(225, 225, 255), sf::Color(180, 180, 180), sf::Color(100, 100, 100));
		clearButton.SetOutline(-2.0f, sf::Color::White);

		text.setFont(AssetHolder::Get().GetFont("sansationRegular"));
		text.setCharacterSize(25);

		level = Level::LoadLevel("files/levels/level0.txt");
		itemMap = Level::LoadLevel("files/levels/level0ItemMap.txt");
	}

	void Input() override {}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		textWindow.ManageEvent(e);

		switch (e.type) {
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::LShift:
				isGridDrawn = !isGridDrawn;
				break;
			}
			break;
		}

		runButton.Logic(e, mousePos);
		clearButton.Logic(e, mousePos);
	}

	void Logic(float dt) override {

		textWindow.Logic();

		if (runButton.GetIsPressed()) {
			isRun = true;
			player.ResetIndex();
			
			player.Run(textWindow.GetStrings(), textWindow.Colors());
		}

		if (clearButton.GetIsPressed()) textWindow.ResetStrings();

		int time = clock.getElapsedTime().asMilliseconds();
		clock.restart();
		t += time;

		if (isRun) {
			if (t > delay) {

				sf::Vector2f playerDirection = player.GetCurrentMovePosition();
				player.Move(level);
				sf::Vector2f playerPos = player.GetPosition();

				for (auto& box : boxes) 
					box.Logic(level, playerPos, playerDirection);

				tile.Logic(boxes);
				t = 0;
			}
		}

		player.Logic(itemMap, t, delay, isRun);

	}

	void SetRect(int x, int y) {
		spriteTile.setTextureRect(sf::IntRect(x * (int)pixelSize, y * (int)pixelSize, (int)pixelSize, (int)pixelSize));
	}

	void Render(sf::RenderWindow& window) override {
		for (int i = 1; i < (int)level.GetHeight() - 1; i++) {
			for (int j = 1; j < (int)level.GetWidth() - 1; j++) {
				switch (level.GetLevel()[i][j]) {
				case '#':
					//pixel.setFillColor(sf::Color::Blue);
					SetRect(1, 1);
					break;
				case '.':
					continue;
					break;
				}
			
				spriteTile.setPosition(j * pixelSize, i * pixelSize);
				window.draw(spriteTile);
			}
		}

		for (int i = 1; i < (int)itemMap.GetHeight() - 1; i++) {
			for (int j = 1; j < (int)itemMap.GetWidth() - 1; j++) {
				switch (itemMap.GetLevel()[i][j]) {
				case '#':
				case '.':
					continue;
					break;
				case '1':
					//circle.setFillColor(sf::Color::Yellow);
					itemTile.setPosition(j * pixelSize, i * pixelSize);
					window.draw(itemTile);
					break;
				case '2':
					pixel.setFillColor(tile.GetIsTileActive() ? sf::Color::Green : sf::Color::Red);
					pixel.setPosition(j * pixelSize, i * pixelSize);
					window.draw(pixel);
					break;
				}
			}
		}
		

		//Grid
		if (isGridDrawn) {
			DrawGrid(window, 1, 1);
		}

		//Player
		pixel.setPosition(player.GetPosition());
		pixel.setFillColor(playerColor);
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
		text.setPosition(windowSize.x - 120.0f, 0.0f);
		window.draw(text);

		//RunButton
		runButton.Render(window);
		
		//Text
		text.setString("Run");
		text.setPosition(50.0f, windowSize.y - 40.0f);
		window.draw(text);

		DrawTextWithValue(window, AssetHolder::Get().GetFont("sansationRegular"), 210.0f, (windowSize.y - 32.0f), "Moves : ", player.GetNMoves(), sf::Color::White, 25);
		
		//ClearButton
		clearButton.Render(window);
	}
};

class Game {
private:
	sf::RenderWindow Window;
	const sf::String windowTitle;

	std::vector<std::unique_ptr<GameState>> gameStates;

	void LoadAssets() {
		AssetHolder::Get().AddFont("sansationRegular", "files/fonts/Sansation_Regular.ttf");
		AssetHolder::Get().AddTexture("Tileset", "files/images/Tileset.png");
		AssetHolder::Get().AddTexture("coin", "files/images/coin.png");
	}

	sf::Clock clock;
	float initDt;
public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: windowTitle(title),
		  Window({ x, y }, title) {
		Window.setFramerateLimit(60);
	
		LoadAssets();
		gameStates.push_back(std::make_unique<PlayState>(sf::Vector2u(x, y)));
	
		initDt = 0.0f;
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