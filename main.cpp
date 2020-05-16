#include <SFML/Graphics.hpp>
#include "GraphicsRender.h"
#include "AssetManager.h"
#include "GraphicsUI.h"
#include <regex>
#include <ctime>

const float pixelSize = 32.0f;

class TextWindow {
private:
	sf::RectangleShape box;

	std::vector<std::string> strings;
	std::ostringstream textString;
	int textIndex;
	sf::Text text;

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
					textString.str(strings[textIndex]);
				}
 			}
		}
		else {
			textString << static_cast<char>(character);
		}
	}
public:
	TextWindow() {}
	TextWindow(const sf::Vector2f& pos, const sf::Vector2f& size) {
		box.setSize(size);
		box.setPosition(pos);
		box.setFillColor(sf::Color(25, 25, 25));
		text.setCharacterSize(15);

		strings.push_back("");
		textIndex = 0;
	}

	void SetFont(const sf::Font& font) {
		text.setFont(font);
	}

	void ResetStrings() {
		textString.str("");
		strings.clear();
		strings.push_back("");
		textIndex = 0;
	}

	inline std::vector<std::string> GetStrings() const { return strings; }

	void AddNewLine() {
		auto it = strings.begin();
		int value = 0;
		while (value != textIndex) {
			it++;
			value++;
		}
		strings.insert(it, textString.str());
		textString.str("");
		textIndex++;
	}

	void ManageEvent(sf::Event e) {
		switch (e.type) {
		case sf::Event::TextEntered:
			if (e.text.unicode < 128) {
				Input(e.text.unicode);
			}
			break;
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::Up:

				strings[textIndex].pop_back();

				textIndex--;
				if (textIndex < 0) textIndex = 0;
				textString.str(strings[textIndex]);
				break;
			case sf::Keyboard::Down:

				strings[textIndex].pop_back();

				textIndex++;
				if (textIndex > (int)(strings.size() - 1)) textIndex = (int)(strings.size() - 1);
				textString.str(strings[textIndex]);
				break;
			case sf::Keyboard::Return:
				AddNewLine();
				break;
			}
			break;
		}

		strings[textIndex] = textString.str() + "_";
	}

	void Render(sf::RenderWindow& window) {
		window.draw(box);
		
		float pos = 2.0f;
		
		for (auto& str : strings) {
			text.setString(sf::String("> ") + str);
			text.setPosition({ 360.0f, pos * (float)text.getCharacterSize() });
			window.draw(text);
			pos++;
		}
	}
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
	int index;
	bool isIndex;

	void IntrepretMovement(const std::vector<std::string>& text, int index) {
		if (text[index + 1] == "Left" || text[index + 1] == "left" || text[index + 1] == "l") {
			AddMovePosition(-1, 0);
		}
		if (text[index + 1] == "Right" || text[index + 1] == "right" || text[index + 1] == "r") {
			AddMovePosition(1, 0);
		}
		if (text[index + 1] == "Up" || text[index + 1] == "up" || text[index + 1] == "u") {
			AddMovePosition(0, -1);
		}
		if (text[index + 1] == "Down" || text[index + 1] == "down" || text[index + 1] == "d") {
			AddMovePosition(0, 1);
		}
	}

	std::size_t IntrepretLoop(const std::vector<std::string>& strings, int nStartPoint, int nLoop) {

		std::vector<std::string> stringsToLoop;
		for (int i = nStartPoint; i < (int)strings.size(); i++) {

			std::string loopString = strings[i];
			while ((loopString.back() == '_' || loopString.back() == ' ') && loopString.size() > 0) loopString.pop_back();

			auto textForLoop = RegexVector(std::regex("\\s"), loopString);

			if (textForLoop.size() > 0 && textForLoop[1] == "closeloop") {
				break;
			}

			stringsToLoop.push_back(loopString);
		}

		for (int n = 0; n < nLoop; n++) {
			for (int i = 0; i < (int)stringsToLoop.size(); i++) {

				std::string initString = stringsToLoop[i];
				if (initString.back() == '_') initString.pop_back();

				auto text = RegexVector(std::regex("\\s"), initString);

				if (text.size() >= 2) {
					for (int j = 0; j < 2; j++) {
						if (text[j] == "Move" || text[j] == "move") {
							IntrepretMovement(text, j);
						}
					}
				}
			}
		}

		return stringsToLoop.size();
	}

	std::vector<std::string> RegexVector(const std::regex& rgx, const std::string& initString) {
		std::vector<std::string> text(
			std::sregex_token_iterator(initString.begin(), initString.end(), rgx, -1),
			std::sregex_token_iterator()
		);

		return text;
	}

public:
	Player() {}
	Player(const sf::Vector2f& pos)
		: playerPos(pos) {
		index = 0;
		isIndex = false;
	}

	void SetPosition(const sf::Vector2f& pos) {
		playerPos = pos;
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
			//if (index > (int)(movePositions.size() - 1)) {
			//	index = 0;
			//	return false;
			//}
		}
	}

	void Run(const std::vector<std::string>& strings) {
		system("cls");
		ClearMovePositions();

		int value = 0, stringIndex = 0;

		for (int i = 0; i < (int)strings.size(); i++) {
			
			//StringIndex ensures that looping strings are not re-read
			stringIndex = i + value;
			if (stringIndex > (int)(strings.size() - 1)) stringIndex = (int)(strings.size() - 1);

			std::string initString = strings[stringIndex];
			if (initString.size() <= 1) continue;
			while ((initString.back() == '_' || initString.back() == ' ') && initString.size() > 0) initString.pop_back();
			
			auto text = RegexVector(std::regex("\\s"), initString);

			//Text strings occupy different indices (index 0 or 1)
			//So check both indices for strings
			if (text.size() >= 2) {
				for (int j = 0; j < 2; j++) {
					if (text[j] == "Move" || text[j] == "move") {
						IntrepretMovement(text, j);
					}
					
					if (text[j] == "startloop" || text[j] == "StartLoop") {
						
						std::istringstream ss(text[j + 1]);
						int n = 0;
						ss >> n;

						//nLines -> number of lines in loop
						int nLines = (int)IntrepretLoop(strings, i, n) - 1;

						value += nLines;
						if (value < 0) value = 0; //If nLines < 0
					}
				}
			}
		}
	}

	void AddMovePosition(int dirX, int dirY) {
		movePositions.emplace_back(dirX, dirY);
	}

	void ClearMovePositions() {
		movePositions.clear();
	}

	inline int GetIndex() const { return index; }
	void ResetIndex() { index = 0; }
	bool& GetIsIndex() { return isIndex; }
	inline sf::Vector2f GetPosition() const { return playerPos; }
	std::vector<sf::Vector2i> GetMovePositions() const { return movePositions; }
};

class PlayState : public GameState {
private:
	Level level;
	sf::RectangleShape pixel;

	Player player;
	sf::Color playerColor;

	TextWindow textWindow;
	Button runButton, clearButton;
	sf::Text text;

	bool isRun;

	sf::Clock clock;
	int t, delay;
public:
	PlayState(const sf::Vector2u& size) 
		: GameState(size) {
		
		textWindow = TextWindow({ (float)size.x - 150.0f, 0.0f }, { 150.0f, (float)size.y });
		textWindow.SetFont(AssetHolder::Get().GetFont("sansationRegular"));

		player = Player({ pixelSize, pixelSize });
		playerColor = sf::Color(rand() % 256, rand() % 256, rand() % 256);

		pixel.setSize({ pixelSize, pixelSize });

		delay = 100; //Milliseconds

		isRun = false;

		runButton.Initialize({ 0.0f, size.y - 50.0f }, { 150.0f, 50.0f });
		runButton.SetColors(sf::Color(90, 100, 200), sf::Color(45, 50, 100), sf::Color(15, 25, 50));
		runButton.SetOutline(-2.0f, sf::Color(rand() % 256, rand() % 256, rand() % 256));

		clearButton.Initialize({ 155.0f, size.y - 50.0f }, { 50.0f, 50.0f });
		clearButton.SetColors(sf::Color(225, 225, 255), sf::Color(180, 180, 180), sf::Color(100, 100, 100));
		clearButton.SetOutline(-2.0f, sf::Color::White);

		text.setFont(AssetHolder::Get().GetFont("sansationRegular"));
		text.setCharacterSize(25);

		level = Level::LoadLevel("files/levels/level0.txt");
	}

	void Input() override {}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		textWindow.ManageEvent(e);

		runButton.Logic(e, mousePos);
		clearButton.Logic(e, mousePos);
	}

	void Logic(float dt) override {
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
				player.Move(level);
				t = 0;
			}
		}

		if (player.GetIndex() > (int)(player.GetMovePositions().size() - 1)) {
			isRun = false;
			player.GetIsIndex() = true;
		}

		if (t > 2 * delay && player.GetIsIndex()) {
			player.GetIsIndex() = false;
			player.SetPosition({ pixelSize, pixelSize });
			t = 0;
		}
	}

	void Render(sf::RenderWindow& window) override {
		for (int i = 0; i < (int)level.GetHeight(); i++) {
			for (int j = 0; j < (int)level.GetWidth(); j++) {
				switch (level.GetLevel()[i][j]) {
				case '#':
					pixel.setFillColor(sf::Color::Blue);
					break;
				case '.':
					continue;
					break;
				}
			
				pixel.setPosition(j * pixelSize, i * pixelSize);
				window.draw(pixel);
			}
		}
		
		pixel.setPosition(player.GetPosition());
		pixel.setFillColor(sf::Color::Red);
		window.draw(pixel);

		textWindow.Render(window);
		DrawLine(window, windowSize.x - 150.0f, 30.0f, (float)windowSize.x, 30.0f);
		text.setString("Console");
		text.setPosition(windowSize.x - 120.0f, 0.0f);
		window.draw(text);

		runButton.Render(window);
		text.setString("Run");
		text.setPosition(50.0f, windowSize.y - 40.0f);
		window.draw(text);

		clearButton.Render(window);
	}
};

class Game {
private:
	sf::RenderWindow Window;
	
	std::vector<std::unique_ptr<GameState>> gameStates;

	void LoadAssets() {
		AssetHolder::Get().AddFont("sansationRegular", "files/fonts/Sansation_Regular.ttf");
	}
public:
	Game(uint32_t x, uint32_t y, const sf::String& title) 
		: Window({ x, y }, title) {
		Window.setFramerateLimit(60);
	
		LoadAssets();
		gameStates.push_back(std::make_unique<PlayState>(sf::Vector2u(x, y)));
	}

	void Logic() {
		while (Window.isOpen()) {
			sf::Event e;

			auto& currentGameState = gameStates.back();

			sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(Window);

			while (Window.pollEvent(e)) {
				switch (e.type) {
				case sf::Event::Closed:
					Window.close();
					break;
				}
			
				currentGameState->ManageEvent(e, mousePos);
			}
		
			currentGameState->Input();
			currentGameState->Logic(1.0f);

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