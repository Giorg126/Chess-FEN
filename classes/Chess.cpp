#include "Chess.h"
#include "Log.h"

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

Chess::Chess(){

}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);
    int gameTag = (playerNumber == 0 ? 0 : 128) + piece;
    bit->setGameTag(gameTag);

    return bit;
}



void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;

    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            piece[0] = bitToPieceNotation(y,x);

            // Log::log(LogLevel::INFO, "Chess Square Notation: " + _grid[y][x].getNotation());
        }
    }

    // initializeBoard();
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    // test boards

    // FENtoBoard("5k2/8/8/8/8/8/8/4K2R w K - 0 1"); // white can castle
    // FENtoBoard("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"); // white can castle queen side
    // FENtoBoard("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1");// white can castle both sides
    // FENtoBoard("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"); // white can promote to queen
    // FENtoBoard("4k3/1P6/8/8/8/8/K7/8 w - - 0 1"); // white can promote to queen

    std::string currentState = stateString();
    Log::log(LogLevel::INFO, "Current State: " + currentState);
}


void Chess::placePieceAt(int playerNumber, ChessPiece piece, int x, int y)
{
    Bit* bit = PieceForPlayer(playerNumber, piece);
    bit->setPosition(_grid[y][x].getPosition());
    bit->setParent(&_grid[y][x]);
    _grid[y][x].setBit(bit);
}

void Chess::initializeBoard()
{
    // pawns
    for (int x = 0; x < 8; x++) {
        placePieceAt(0, Pawn, x, 1);
        placePieceAt(1, Pawn, x, 6);
    }

    // rooks
    placePieceAt(0, Rook, 0, 0);
    placePieceAt(0, Rook, 7, 0);
    placePieceAt(1, Rook, 0, 7);
    placePieceAt(1, Rook, 7, 7);

    // knights
    placePieceAt(0, Knight, 1, 0);
    placePieceAt(0, Knight, 6, 0);
    placePieceAt(1, Knight, 1, 7);
    placePieceAt(1, Knight, 6, 7);

    // bishops
    placePieceAt(0, Bishop, 2, 0);
    placePieceAt(0, Bishop, 5, 0);
    placePieceAt(1, Bishop, 2, 7);
    placePieceAt(1, Bishop, 5, 7);

    // queens
    placePieceAt(0, Queen, 3, 0);
    placePieceAt(1, Queen, 3, 7);

    // kings
    placePieceAt(0, King, 4, 0);
    placePieceAt(1, King, 4, 7);
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return true;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src) {
    Player *currentPlayer = getCurrentPlayer();
    int currentPlayerNumber = currentPlayer->playerNumber();

    if ((bit.gameTag() < 128 && currentPlayerNumber != 0) || 
        (bit.gameTag() >= 128 && currentPlayerNumber != 1)) {
        return false; 
    }
    return true;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {

    // retrieve source and destination row and column indices
    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);

    int srcRow = srcSquare.getRow();
    int srcCol = srcSquare.getColumn();
    int dstRow = dstSquare.getRow();
    int dstCol = dstSquare.getColumn();

    // step length in coordinates
    int rowDiff = abs(dstRow - srcRow);
    int colDiff = abs(dstCol - srcCol);

    // check the direction of the move
    bool isStraightMove = (srcRow == dstRow || srcCol == dstCol);
    bool isDiagonalMove = (rowDiff == colDiff);
    bool isKnightMove = (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);

    // avoid moving on a square containing an ally piece
    if (dst.bit() != nullptr && dst.bit()->getOwner() == bit.getOwner()) {
        return false; 
    }

    // pieces specific moves

    if (bit.gameTag() % 128 == Queen) {
        if (!isStraightMove && !isDiagonalMove) {
            return false; 
        }
    }
    if (bit.gameTag() % 128 == Rook) {
        if (!isStraightMove) {
            return false; 
        }
    }
    if (bit.gameTag() % 128 == Bishop) {
        if (!isDiagonalMove) {
            return false; 
        }
    }
    if (bit.gameTag() % 128 == King) {
        // castling
        if (_gameOptions.canWhiteCastleKingside) {
            if (srcRow == 0 && srcCol == 4 && dstRow == 0 && dstCol == 6) {
                if (_grid[0][5].bit() == nullptr && _grid[0][6].bit() == nullptr) {
                    return true; 
                }
            }
        }
        if (_gameOptions.canWhiteCastleQueenside) {
            if (srcRow == 0 && srcCol == 4 && dstRow == 0 && dstCol == 2) {
                if (_grid[0][1].bit() == nullptr && _grid[0][2].bit() == nullptr && _grid[0][3].bit() == nullptr) {
                    return true; 
                }
            }
        }
        if (_gameOptions.canBlackCastleKingside) {
            if (srcRow == 7 && srcCol == 4 && dstRow == 7 && dstCol == 6) {
                if (_grid[7][5].bit() == nullptr && _grid[7][6].bit() == nullptr) {
                    return true; 
                }
            }
        }
        if (_gameOptions.canBlackCastleQueenside) {
            if (srcRow == 7 && srcCol == 4 && dstRow == 7 && dstCol == 2) {
                if (_grid[7][1].bit() == nullptr && _grid[7][2].bit() == nullptr && _grid[7][3].bit() == nullptr) {
                    return true; 
                }
            }
        }
        // normal king move
        if (rowDiff > 1 || colDiff > 1) {
            return false; 
        }
    }
    if (bit.gameTag() % 128 == Knight) {
        if (!((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2))) {
            return false; 
        }
    }
    if (bit.gameTag() % 128 == Pawn) {
        // a pawn has a maximum of 2 steps forward and 1 step diagonally
        if (colDiff > 1 || rowDiff > 2 || rowDiff == 0)
            return false;

        // a pawn can't move backwards!
        if (bit.gameTag() < 128 && dstRow < srcRow)
            return false;
        if (bit.gameTag() >= 128 && dstRow > srcRow)
            return false;
        
        // allow 2 steps only if it's the first move
        if (rowDiff > 1) {
            if (colDiff != 0)
                return false;
            else 
            if ((bit.gameTag() < 128 && srcRow != 1) || (bit.gameTag() >= 128 && srcRow != 6) || dst.bit() != nullptr)
                return false;
            }
        
        if (rowDiff == 1) {
            // avoid eating forward
            if (colDiff == 0) {
                if (dst.bit() != nullptr) 
                    return false;
                
            // allow eating diagonally
            } else {
                if (dst.bit() == nullptr) 
                    // compare the destination square with the en passant square
                    if (_gameOptions.enPassantSquare != dstSquare.getNotation())
                        return false;
            }
        }
    }

    // check if the path is clear
    int rowStep = (dstRow > srcRow) ? 1 : ((dstRow < srcRow) ? -1 : 0);
    int colStep = (dstCol > srcCol) ? 1 : ((dstCol < srcCol) ? -1 : 0);

    int currentRow = srcRow + rowStep;
    int currentCol = srcCol + colStep;

    // if it's a knight no need to check the path
    if (isKnightMove) {
        return true; 
    }

    while (currentRow != dstRow || currentCol != dstCol){
        if (_grid[currentRow][currentCol].bit() != nullptr) {
            return false; 
        }
        currentRow += rowStep;
        currentCol += colStep;
    }

    return true;
}





void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    endTurn();

    ChessSquare &srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare&>(dst);

    // eat the piece in case of en passant befotre updating the en passant square
    if (bit.gameTag() % 128 == Pawn && dstSquare.getNotation() == _gameOptions.enPassantSquare) {
        int captureRow = (bit.gameTag() < 128) ? dstSquare.getRow() - 1 : dstSquare.getRow() + 1;
        ChessSquare &capturedSquare = static_cast<ChessSquare&>(getHolderAt(dstSquare.getColumn(), captureRow));
        pieceTaken(capturedSquare.bit());
        capturedSquare.setBit(nullptr);
    }

    // move the rook in case of castling
    if (bit.gameTag() % 128 == King && abs(srcSquare.getColumn() - dstSquare.getColumn()) == 2) {
        int rookCol = (dstSquare.getColumn() == 6) ? 7 : 0;
        ChessSquare &rookSquare = static_cast<ChessSquare&>(getHolderAt(rookCol, dstSquare.getRow()));
        ChessSquare &newRookSquare = static_cast<ChessSquare&>(getHolderAt((dstSquare.getColumn() == 6) ? 5 : 3, dstSquare.getRow()));
        placePieceAt(bit.getOwner()->playerNumber(), Rook, newRookSquare.getColumn(), newRookSquare.getRow());
        rookSquare.setBit(nullptr);
    }

    // update castling rights only if there's at least one boolean true
    if (_gameOptions.canWhiteCastleKingside || _gameOptions.canWhiteCastleQueenside || _gameOptions.canBlackCastleKingside || _gameOptions.canBlackCastleQueenside)
        updateCastlingRights();

    // update the en passant square
    if (bit.gameTag() % 128 == Pawn && abs(srcSquare.getRow() - dstSquare.getRow()) == 2) {
        if (bit.gameTag() < 128) {
            _gameOptions.enPassantSquare = _grid[dstSquare.getRow() - 1][dstSquare.getColumn()].getNotation();
        } else {
            _gameOptions.enPassantSquare = _grid[dstSquare.getRow() + 1][dstSquare.getColumn()].getNotation();
        }
    } else {
        _gameOptions.enPassantSquare = "-";
    }

    // promotion
    if (bit.gameTag() % 128 == Pawn && (dstSquare.getRow() == 0 || dstSquare.getRow() == 7)) {
        // promote the pawn to a queen
        placePieceAt(bit.getOwner()->playerNumber(), Queen, dstSquare.getColumn(), dstSquare.getRow());
    }

    // full moves
    if (bit.getOwner()->playerNumber() == 1) {
        _gameOptions.fullMoves++;
    }

    std::string currentState = stateString();
    Log::log(LogLevel::INFO, "Current State: " + currentState);
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    // check to see if either player has won
    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
    return false;
}

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        int empty = 0;
        for (int x = 0; x < _gameOptions.rowX; x++) {
            std::string piece(1,bitToPieceNotation(y, x));
            if (piece != "0") {
                if (empty) {
                    s += std::to_string(empty);
                    empty = 0;
                }
                s += piece;
            } else {
                empty++;
            }
        }
        if (empty) {
            s += std::to_string(empty);
        }
        if (y < _gameOptions.rowY - 1) {
            s += "/";
        }
    }

    // return s without the last /
    // return s.substr(0, s.size() - 1);

    // add the player to move
    s += " ";
    s += (getCurrentPlayer()->playerNumber() == 0) ? "w" : "b";

    // add the castling rights
    s += " ";
    if (!_gameOptions.canWhiteCastleKingside && !_gameOptions.canWhiteCastleQueenside) { 
        s += "-";
    }
    else {
        if (_gameOptions.canWhiteCastleKingside) {
            s += "K";
        }
        if (_gameOptions.canWhiteCastleQueenside) {
            s += "Q";
        }
    }

    if (!_gameOptions.canBlackCastleKingside && !_gameOptions.canBlackCastleQueenside) { 
        s += "-";
    }
    else {
        if (_gameOptions.canBlackCastleKingside) {
            s += "k";
        }
        if (_gameOptions.canBlackCastleQueenside) {
            s += "q";
        }
    }

    // add the en passant square
    s += " ";
    s += _gameOptions.enPassantSquare;

    // add the halfmove clock
    s += " ";
    s += std::to_string(_gameOptions.halfMoves);

    // add the fullmove number
    s += " ";
    s += std::to_string(_gameOptions.fullMoves);

    return s;
}

//
// for now this function only checks if a king or a tower has moved
// the holder position is sufficient to determine if a king or a tower has moved for the first time
void Chess::updateCastlingRights() {

    // white
    if (!_grid[0][4].bit() || _grid[0][4].bit()->gameTag() % 128 != King || _grid[0][4].bit()->gameTag() >= 128) {
        _gameOptions.canWhiteCastleKingside = false;
        _gameOptions.canWhiteCastleQueenside = false;
    }
    if (!_grid[0][0].bit() || _grid[0][0].bit()->gameTag() % 128 != Rook || _grid[0][0].bit()->gameTag() >= 128) {
        _gameOptions.canWhiteCastleQueenside = false;
    }
    if (!_grid[0][7].bit() || _grid[0][7].bit()->gameTag() % 128 != Rook || _grid[0][7].bit()->gameTag() >= 128) {
        _gameOptions.canWhiteCastleKingside = false;
    }

    // black pieces
    if (!_grid[7][4].bit() || _grid[7][4].bit()->gameTag() % 128 != King || _grid[7][4].bit()->gameTag() < 128) {
        _gameOptions.canBlackCastleKingside = false;
        _gameOptions.canBlackCastleQueenside = false;
    }
    if (!_grid[7][0].bit() || _grid[7][0].bit()->gameTag() % 128 != Rook || _grid[7][0].bit()->gameTag() < 128) {
        _gameOptions.canBlackCastleQueenside = false;
    }
    if (!_grid[7][7].bit() || _grid[7][7].bit()->gameTag() % 128 != Rook || _grid[7][7].bit()->gameTag() < 128) {
        _gameOptions.canBlackCastleKingside = false;
    }
}


// function to count the number of half moves since the last capture or pawn move
void Chess::updateHalfMoves(Bit &bit, BitHolder &src, BitHolder &dst) {
    if (bit.gameTag() % 128 == Pawn || dst.bit() != nullptr) {
        _gameOptions.halfMoves = 0;
    } else {
        _gameOptions.halfMoves++;
    }
}


void Chess::FENtoBoard(const std::string &fen)
{
    int row = 0;
    int column = 0;

    size_t spacePos = fen.find(" ");

    std::string boardPosition = fen.substr(0, spacePos);

    for (int i = 0; i < boardPosition.size(); i++) {
        char c = fen[i];
        if (c == '/') {
            row++;
            column = 0;
        } else {
            if (c >= '0' && c <= '9') {
                column += c - '0';
            } else {
                int playerNumber = (c >= 'a' && c <= 'z') ? 1 : 0;
                int piece = 0;
                switch (c) {
                    case 'p':
                    case 'P':
                        piece = Pawn;
                        break;
                    case 'n':
                    case 'N':
                        piece = Knight;
                        break;
                    case 'b':
                    case 'B':
                        piece = Bishop;
                        break;
                    case 'r':
                    case 'R':
                        piece = Rook;
                        break;
                    case 'q':
                    case 'Q':
                        piece = Queen;
                        break;
                    case 'k':
                    case 'K':
                        piece = King;
                        break;
                }
                int reversedRow = _gameOptions.rowY - 1 - row;
                placePieceAt(playerNumber, (ChessPiece)piece, column, reversedRow);
                column++;
            }
        }
    }
    if (spacePos != std::string::npos) {
    std::string gameState = fen.substr(spacePos + 1);

    // retrieve different strings separated by spaces
    std::vector<std::string> fields;
    size_t start = 0, end;
    while ((end = gameState.find(" ", start)) != std::string::npos) {
        fields.push_back(gameState.substr(start, end - start));
        start = end + 1;
    }
    if (start < gameState.size()) { 
        fields.push_back(gameState.substr(start));
    }

    if (fields.size() > 0) { // current player
        _gameOptions.currentTurnNo = (fields[0] == "w") ? 0 : 1; 
    }
    if (fields.size() > 1) { // castling rights
        std::string castlingRights = fields[1];
        _gameOptions.canWhiteCastleKingside = castlingRights.find('K') != std::string::npos;
        _gameOptions.canWhiteCastleQueenside = castlingRights.find('Q') != std::string::npos;
        _gameOptions.canBlackCastleKingside = castlingRights.find('k') != std::string::npos;
        _gameOptions.canBlackCastleQueenside = castlingRights.find('q') != std::string::npos;
    }
    if (fields.size() > 2) { // en passant case
        _gameOptions.enPassantSquare = fields[2]; 
    }
    if (fields.size() > 3) { // half moves
        _gameOptions.halfMoves = std::stoi(fields[3]);
    }
    if (fields.size() > 4) { // full moves
        _gameOptions.fullMoves = std::stoi(fields[4]); 
    }
    } else {
       _gameOptions.currentTurnNo = 0;
        _gameOptions.canWhiteCastleKingside = true;
        _gameOptions.canWhiteCastleQueenside = true;
        _gameOptions.canBlackCastleKingside = true;
        _gameOptions.canBlackCastleQueenside = true;
        updateCastlingRights();
        _gameOptions.enPassantSquare = "-";
        _gameOptions.halfMoves = 0;
        _gameOptions.fullMoves = 0;

    }
}


//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}


//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
}

