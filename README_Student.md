Giorgio Vallesi - Add setup from FEN, Castling, Promotion to Queen, and En Passant to your move generation code

Added gameOptions to Game class, booleans for the castling rights, counters for half and full moves count, string for en passant case. Also changed some lines the Game class to allow a fair half moves counting and to highlight the possible moves once a piece is selected.

updateCastlingRights: The function simply looks at the board, and if the kings and rooks are not in their initial positions, updates the booleans to false. Calling it every turn it should be enough to state if pieces have moved or a rook has been captured.

FENtoBoard: Now supports board initialization from a FEN string, allows long detailed form or short form. The stateString function will understand the other FEN information looking at the board. Due to updateCastlingRights, if rooks and kings are in position the function will return true for the booleans (for extra information, for example rook has moved and then is back in position, one should give the full detailed form of FEN as input)

Special Moves
-Castling: if the booleans are true and the inn between cases are free, the king can castle.
-En Passant: at every turn if a pawn has long-moved, the en passant position is stored in the gameOptions.enPassantSquare, or a null value in case another move has been performed. If a pawn is in the right position, it can correctly en passant.
-Pawn Promotion: awn automatically promote to queen when reaching last raw.
Castling Checks: To determine if castling is available, new booleans were added to track whether the king or relevant rooks have moved. This ensures that castling is only permitted under valid conditions.
