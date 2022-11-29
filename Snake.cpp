#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <ctime>
#include <cstdlib>

using namespace std;

/* A struct encoding a point on a two-dimensional grid*/
struct pointT {
    int row, col;
};

/* A struct containing relevant game information */
struct gameT {
    vector<string> world;       // The playing field
    int numRows, numCols;       // Size of the playing field

    deque<pointT> snake;        // The snake body
    int dx, dy;                 // The snake direction

    int numEaten;               // Number of pellets eaten
};

/* Function declarations */
void OpenUserFile(ifstream& input);
void InitializeGame(gameT& game);
void RunSimulation(gameT& game);
pointT MakePoint(int row, int col);
void LoadWorld(gameT& game, ifstream& input);
void PrintWorld(gameT& game);
void PerformAI(gameT& game);
bool MoveSnake(gameT& game);
pointT GetNextPosition(gameT& game, int dx, int dy);
bool InWorld(pointT& pt, gameT& game);
bool Crashed(pointT headPos, gameT& game);
bool RandomChance(double probability);
void Pause();
void DisplayResults(gameT& game);
void PlaceFood(gameT& game);
string GetLine();

/* Constants */
const int kMaxFood = 20;               // Number of food pellets required to win 
const double kWaitTime = 0.1;          // Pause 0.1 seconds between frames
const string kClearCommand = "clear";

/* Constants for different tile types */
const char kEmptyTile = ' ';
const char kWallTile = '#';
const char kFoodTile = '$';
const char kSnakeTile = '*';
const double kTurnRate = 0.2;

void OpenUserFile(ifstream& input) {
    while(true) {
        cout << "Enter filename:  ";
        string filename = GetLine();

        input.open(filename.c_str());       
        if(input.is_open()) break;

        cout << "Sorry, I can't find the file " << filename << endl;
        input.clear() ;
    }
}


void LoadWorld(gameT& game, ifstream& input) {
    input >> game.numRows >> game.numCols;
    game.world.resize(game.numRows);

    input >> game.dx >> game.dy;

    string dummy;
    getline(input, dummy);

    for(int row = 0; row < game.numRows; ++row) {
        getline(input, game.world[row]);
        int col = game.world[row].find(kSnakeTile);
        if(col != string::npos) {
            game.snake.push_back(MakePoint(row, col));
        }
    }
    game.numEaten = 0;
}

void InitializeGame(gameT& game) {
    /* Seed the randomizer.  The static cast converts the result of time(NULL) from 
    time_t to the unsigned in required by srand. */
    srand(static_cast<unsigned int>(time(NULL)));
    ifstream input;
    OpenUserFile(input);
    LoadWorld(game, input);
}

void RunSimulation(gameT& game) {
    /* Keep looping while we haven't eaten too much */
    while(game.numEaten < kMaxFood) {
        PrintWorld(game);               // Display the board
        PerformAI(game);                // Have the AI choose an action

        if(!MoveSnake(game))            // Move the snake and stop if we crashed
            break;

        Pause();                        // Pause so we can see what's going on
    }   
    DisplayResults(game);               // Tell us what happened
}

string GetLine() {
    string result;
    getline(cin, result);
    return result;
}


pointT MakePoint(int row, int col) {
    pointT result;
    result.row = row;
    result.col = col;
    return result;
}


void PrintWorld(gameT& game) {
    system(kClearCommand.c_str());
    for(int row = 0; row < game.numRows; ++row) {
        cout << game.world[row] << endl;
    }
    cout << "Food eaten: " << game.numEaten << endl;
}

pointT GetNextPosition(gameT& game, int dx, int dy) {
    /* Get the head position */
    pointT result = game.snake.front();

    /* Increment the head position by the current direction */
    result.row += dy;
    result.col += dx;
    return result;
}

bool InWorld(pointT& pt, gameT& game) {
    return pt.col >= 0 && 
           pt.row >= 0 &&
           pt.col < game.numCols &&
           pt.row < game.numRows;
}

bool Crashed(pointT headPos, gameT& game) {
    return !InWorld(headPos, game) || 
        game.world[headPos.row][headPos.col] == kSnakeTile ||
        game.world[headPos.row][headPos.col] == kWallTile; 
}

bool RandomChance(double probability) {
    return (rand() / (RAND_MAX + 1.0)) < probability;
}

void PerformAI(gameT& game) {
    /* Figure out where we will be after we move this turn*/
    pointT nextHead = GetNextPosition(game, game.dx, game.dy);

    /* It that hits a wall or we reandomly decide to, turn the snake */
    if(Crashed(nextHead, game) || RandomChance(kTurnRate)) {
        int leftDx = -game.dy;
        int leftDy = game.dx;

        int rightDx = game.dy;
        int rightDy = -game.dx;

        /* Check if turning left or right will cause us to crash */
        bool canLeft = !Crashed(GetNextPosition(game, leftDx, leftDy), game);
        bool canRight = !Crashed(GetNextPosition(game, rightDx, rightDy), game);

        bool willTurnLeft = false;
        if(!canLeft && !canRight) return;  // If we can't turn, don't turn
        else if(canLeft && !canRight) willTurnLeft =  true;  // If must turn left, do so
        else if(!canLeft && canRight) willTurnLeft = false;  // If must turn right, do so
        else willTurnLeft =  RandomChance(0.5);  // Else pick randomly

        game.dx = willTurnLeft? leftDx :  rightDx;
        game.dy = willTurnLeft? leftDy : rightDy;

    }
}


void PlaceFood(gameT& game) {
    while(true) {
        int row = rand() % game.numRows;
        int col = rand() % game.numCols;

        /* If the specified position is empty, place the food there */
        if(game.world[row][col] == kEmptyTile) {
            game.world[row][col] == kFoodTile;
            return;
        }
    }
}

bool MoveSnake(gameT& game) {
    pointT nextHead = GetNextPosition(game, game.dx, game.dy);
    if(Crashed(nextHead, game)) return false;

    bool isFood = (game.world[nextHead.row][nextHead.col] == kFoodTile);

    game.world[nextHead.row][nextHead.col] = kSnakeTile;
    game.snake.push_front(nextHead);

    if(!isFood) {
        game.world[game.snake.back().row][game.snake.back().col] = kEmptyTile;
        game.snake.pop_back();
    } else {
        ++game.numEaten;
        PlaceFood(game);
    }
    return true;
}

void Pause() {
    clock_t startTime = clock();    // clock_t is a type which holds clock ticks

    /* This loop does nothing except loop and check how much time is left. 
    startTime is typecast from a clock_t into a double for precise deivion.*/

    while(static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC < kWaitTime);
}


void DisplayResults(gameT& game) {
    PrintWorld(game);
    if(game.numEaten == kMaxFood) {
        cout << " The snake ate enough food and wins! " << endl;
    } else {
        cout << " Oh no.  The snake crashed! " << endl;
    }
}

/* Main program.  Initializes game and runs simulation */
int main() {
    gameT game;
    InitializeGame(game);
    RunSimulation(game);
    return 0;
}