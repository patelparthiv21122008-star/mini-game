// ============================================================
//  Mini Arcade — Console Games: Tic-Tac-Toe & Snake
//  Demonstrates: loops, arrays, conditionals, game logic
//  Compile:  g++ -std=c++17 -o mini_games mini_games.cpp
//  Windows:  add  -lws2_32  or use <conio.h> for kbhit()
// ============================================================
#include <iostream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  void clearScreen()  { system("cls"); }
  void hideCursor()   { HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); CONSOLE_CURSOR_INFO i{1,FALSE}; SetConsoleCursorInfo(h,&i); }
  char getChar()      { return _getch(); }
  bool keyAvailable() { return _kbhit(); }
#else
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>
  void clearScreen()  { system("clear"); }
  void hideCursor()   { std::cout << "\033[?25l"; }
  void showCursor()   { std::cout << "\033[?25h"; }

  struct TermRAII {
    termios old;
    TermRAII()  { tcgetattr(STDIN_FILENO, &old); termios t = old; t.c_lflag &= ~(ICANON|ECHO); tcsetattr(STDIN_FILENO, TCSANOW, &t); fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); }
    ~TermRAII() { tcsetattr(STDIN_FILENO, TCSANOW, &old); fcntl(STDIN_FILENO, F_SETFL, 0); showCursor(); }
  };

  char getChar() { char c=0; read(STDIN_FILENO,&c,1); return c; }
  bool keyAvailable() { char c=0; int n=read(STDIN_FILENO,&c,1); if(n>0){ungetc(c,stdin);return true;}return false; }
#endif

// ─── Colours (ANSI) ─────────────────────────────────────────
namespace Col {
  const std::string RST  = "\033[0m";
  const std::string BOLD = "\033[1m";
  const std::string CYN  = "\033[96m";
  const std::string MAG  = "\033[95m";
  const std::string GRN  = "\033[92m";
  const std::string YEL  = "\033[93m";
  const std::string RED  = "\033[91m";
  const std::string GRY  = "\033[90m";
}

// ============================================================
//  TIC-TAC-TOE
// ============================================================
namespace TicTacToe {

  using Board = std::array<char, 9>;

  const int WIN_LINES[8][3] = {
    {0,1,2},{3,4,5},{6,7,8},   // rows
    {0,3,6},{1,4,7},{2,5,8},   // cols
    {0,4,8},{2,4,6}            // diagonals
  };

  void drawBoard(const Board& b, const int* highlight = nullptr) {
    clearScreen();
    std::cout << Col::BOLD << Col::CYN
              << "\n  ╔═══════════╗\n"
              << "  ║ TIC-TAC-TOE ║\n"
              << "  ╚═══════════╝\n\n" << Col::RST;

    std::cout << Col::GRY << "  [1][2][3]  ← position numbers\n\n" << Col::RST;

    for (int r = 0; r < 3; ++r) {
      std::cout << "  ";
      for (int c = 0; c < 3; ++c) {
        int idx = r * 3 + c;
        bool isWin = false;
        if (highlight) for (int k = 0; k < 3; ++k) isWin |= (highlight[k] == idx);

        if (c > 0) std::cout << Col::GRY << "│" << Col::RST;

        std::string cell(1, b[idx]);
        if (b[idx] == 'X')
          std::cout << (isWin ? Col::GRN : Col::CYN) << " X " << Col::RST;
        else if (b[idx] == 'O')
          std::cout << (isWin ? Col::GRN : Col::MAG) << " O " << Col::RST;
        else
          std::cout << Col::GRY << " " << (idx+1) << " " << Col::RST;
      }
      std::cout << "\n";
      if (r < 2) std::cout << "  " << Col::GRY << "───┼───┼───\n" << Col::RST;
    }
    std::cout << "\n";
  }

  // Returns winning line index array or nullptr
  const int* checkWin(const Board& b, char player) {
    static int line[3];
    for (auto& wl : WIN_LINES) {
      if (b[wl[0]] == player && b[wl[1]] == player && b[wl[2]] == player) {
        line[0]=wl[0]; line[1]=wl[1]; line[2]=wl[2];
        return line;
      }
    }
    return nullptr;
  }

  bool boardFull(const Board& b) {
    for (char c : b) if (c == ' ') return false;
    return true;
  }

  void run() {
    int scores[2] = {0, 0};  // X, O
    int draws = 0;
    bool playing = true;

    while (playing) {
      Board board;
      board.fill(' ');
      char currentPlayer = 'X';
      bool gameOver = false;

      while (!gameOver) {
        drawBoard(board);

        // Scoreboard
        std::cout << Col::YEL << "  Scores → "
                  << Col::CYN  << "X: " << scores[0]
                  << Col::GRY  << " | "
                  << Col::MAG  << "O: " << scores[1]
                  << Col::GRY  << " | "
                  << "Draws: " << draws << Col::RST << "\n\n";

        std::cout << (currentPlayer == 'X' ? Col::CYN : Col::MAG)
                  << "  Player " << currentPlayer << "'s turn. Enter 1-9: "
                  << Col::RST;

        int move;
        std::cin >> move;

        if (std::cin.fail() || move < 1 || move > 9 || board[move-1] != ' ') {
          std::cin.clear();
          std::cin.ignore(1000, '\n');
          std::cout << Col::RED << "  Invalid move! Try again.\n" << Col::RST;
          std::this_thread::sleep_for(std::chrono::milliseconds(900));
          continue;
        }

        board[move-1] = currentPlayer;

        const int* winLine = checkWin(board, currentPlayer);
        if (winLine) {
          drawBoard(board, winLine);
          std::cout << (currentPlayer=='X' ? Col::CYN : Col::MAG)
                    << Col::BOLD << "  🎉 Player " << currentPlayer << " WINS!\n"
                    << Col::RST;
          scores[currentPlayer == 'X' ? 0 : 1]++;
          gameOver = true;
        } else if (boardFull(board)) {
          drawBoard(board);
          std::cout << Col::YEL << Col::BOLD << "  It's a DRAW!\n" << Col::RST;
          draws++;
          gameOver = true;
        } else {
          currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
        }
      }

      std::cout << "\n  Play again? (y/n): ";
      char ch; std::cin >> ch;
      playing = (ch == 'y' || ch == 'Y');
    }
  }
}

// ============================================================
//  SNAKE
// ============================================================
namespace Snake {

  const int W = 20, H = 18;  // grid dimensions

  struct Point { int x, y; };

  enum Dir { UP, DOWN, LEFT, RIGHT };

  void draw(const std::vector<Point>& body, Point food, int score, int hi, bool dead) {
    clearScreen();
    std::cout << Col::BOLD << Col::GRN
              << "\n  ╔══════════╗\n"
              << "  ║   SNAKE   ║\n"
              << "  ╚══════════╝\n" << Col::RST;
    std::cout << Col::YEL << "  Score: " << score
              << "   Best: " << hi << Col::RST << "\n\n";

    // Top wall
    std::cout << "  " << Col::GRY;
    for (int x = 0; x < W+2; ++x) std::cout << "█";
    std::cout << Col::RST << "\n";

    // Grid rows
    for (int y = 0; y < H; ++y) {
      std::cout << "  " << Col::GRY << "█" << Col::RST;
      for (int x = 0; x < W; ++x) {
        Point p{x, y};
        bool isHead = !body.empty() && body[0].x == x && body[0].y == y;
        bool isBody = false;
        for (size_t i = 1; i < body.size(); ++i)
          if (body[i].x == x && body[i].y == y) { isBody = true; break; }
        bool isFood = (food.x == x && food.y == y);

        if (isHead)       std::cout << (dead ? Col::RED : Col::CYN) << "◉" << Col::RST;
        else if (isBody)  std::cout << Col::GRN << "●" << Col::RST;
        else if (isFood)  std::cout << Col::MAG << "♦" << Col::RST;
        else              std::cout << " ";
      }
      std::cout << Col::GRY << "█" << Col::RST << "\n";
    }

    // Bottom wall
    std::cout << "  " << Col::GRY;
    for (int x = 0; x < W+2; ++x) std::cout << "█";
    std::cout << Col::RST << "\n\n";

    std::cout << Col::GRY << "  Controls: W/A/S/D  |  Q = quit\n" << Col::RST;

    if (dead) {
      std::cout << "\n" << Col::RED << Col::BOLD
                << "  ★ GAME OVER! Final score: " << score << " ★\n"
                << Col::RST;
    }
  }

  Point randomFood(const std::vector<Point>& body) {
    Point f;
    do {
      f = {rand() % W, rand() % H};
    } while ([&](){
      for (auto& s : body) if (s.x==f.x && s.y==f.y) return true;
      return false;
    }());
    return f;
  }

  void run() {
    int hiScore = 0;
    bool playing = true;

#ifndef _WIN32
    TermRAII term;
#endif
    hideCursor();

    while (playing) {
      std::vector<Point> body = {{W/2, H/2}, {W/2-1, H/2}, {W/2-2, H/2}};
      Dir dir = RIGHT, nextDir = RIGHT;
      Point food = randomFood(body);
      int score = 0;
      bool alive = true;

      while (alive) {
        // Non-blocking input
        if (keyAvailable()) {
          char c = getChar();
          if      ((c=='w'||c=='W') && dir!=DOWN)  nextDir = UP;
          else if ((c=='s'||c=='S') && dir!=UP)    nextDir = DOWN;
          else if ((c=='a'||c=='A') && dir!=RIGHT) nextDir = LEFT;
          else if ((c=='d'||c=='D') && dir!=LEFT)  nextDir = RIGHT;
          else if (c=='q'||c=='Q')  { alive = false; playing = false; break; }
          // Handle arrow keys (escape sequences)
          else if (c == '\033') {
            getChar(); // '['
            char arrow = getChar();
            if      (arrow=='A' && dir!=DOWN)  nextDir = UP;
            else if (arrow=='B' && dir!=UP)    nextDir = DOWN;
            else if (arrow=='C' && dir!=LEFT)  nextDir = RIGHT;
            else if (arrow=='D' && dir!=RIGHT) nextDir = LEFT;
          }
        }
        dir = nextDir;

        // Move head
        Point newHead = body.front();
        if      (dir == UP)    --newHead.y;
        else if (dir == DOWN)  ++newHead.y;
        else if (dir == LEFT)  --newHead.x;
        else if (dir == RIGHT) ++newHead.x;

        // Collision — walls
        if (newHead.x < 0 || newHead.x >= W || newHead.y < 0 || newHead.y >= H) {
          alive = false; break;
        }
        // Collision — self
        for (auto& s : body) {
          if (s.x == newHead.x && s.y == newHead.y) { alive = false; break; }
        }
        if (!alive) break;

        body.insert(body.begin(), newHead);

        // Ate food?
        if (newHead.x == food.x && newHead.y == food.y) {
          score++;
          if (score > hiScore) hiScore = score;
          food = randomFood(body);
        } else {
          body.pop_back();
        }

        draw(body, food, score, hiScore, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(140));
      }

      if (!playing) break;

      draw(body, food, score, hiScore, true);
      std::cout << "\n  Play again? (y/n): ";

      // Re-enable blocking input for the prompt
#ifndef _WIN32
      termios t; tcgetattr(STDIN_FILENO, &t);
      t.c_lflag |= (ICANON|ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &t);
      fcntl(STDIN_FILENO, F_SETFL, 0);
#endif
      char ch; std::cin >> ch;
      playing = (ch == 'y' || ch == 'Y');

#ifndef _WIN32
      tcgetattr(STDIN_FILENO, &t);
      t.c_lflag &= ~(ICANON|ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &t);
      fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
#endif
    }

#ifdef _WIN32
    ShowCursor(TRUE);
#endif
  }
}

// ============================================================
//  MAIN MENU
// ============================================================
int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  while (true) {
    clearScreen();
    std::cout << Col::BOLD << Col::YEL
              << "\n  ╔═══════════════════════╗\n"
              << "  ║    ★  MINI  ARCADE  ★  ║\n"
              << "  ╚═══════════════════════╝\n\n"
              << Col::RST;
    std::cout << Col::CYN  << "  [1]  Tic-Tac-Toe\n" << Col::RST;
    std::cout << Col::GRN  << "  [2]  Snake\n"        << Col::RST;
    std::cout << Col::GRY  << "  [0]  Quit\n\n"       << Col::RST;
    std::cout << "  Choose: ";

    int choice;
    std::cin >> choice;
    if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(1000,'\n'); continue; }

    if      (choice == 1) TicTacToe::run();
    else if (choice == 2) Snake::run();
    else if (choice == 0) break;
  }

  clearScreen();
  std::cout << Col::YEL << Col::BOLD
            << "\n  Thanks for playing! 👾\n\n"
            << Col::RST;
  return 0;
}
