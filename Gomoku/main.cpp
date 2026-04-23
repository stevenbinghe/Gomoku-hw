#include<iostream>
#include<string>
#include<vector>
#include<algorithm>


const int BoardSize = 38;
enum Cell { EMPTY = 0, BLACK = 1, WHITE = 2 };
const int INF = 1e9;


class Piece
{
public:
	int a, b;
	Piece()
		:a(-1),b(-1){}
	Piece(int a, int b)
		:a(a),b(b){ }
	Piece operator+(const Piece& other)const
	{
		return Piece(a + other.a, b + other.b);
	}
	Piece operator-()const
	{
		return Piece(-a,-b);
	}
	Piece operator*(int num)const
	{
		return Piece(num * a, num * b);
	}
};
std::ostream& operator<<(std::ostream& stream, const Piece& other)
{
	stream << other.a << "," << other.b;
	return stream;
}

const Piece dir_piece[4] = { Piece(1,0),Piece(0,1),Piece(1,1),Piece(1,-1) };

class Board {
public:
	int grid[BoardSize][BoardSize] = { EMPTY };

	std::vector<Piece> history;

	Board()
	{
		ReSet();
	}
	bool IsInside(const Piece& piece) const
	{
		return piece.a < BoardSize && piece.b < BoardSize && piece.a>-1 && piece.b>-1;
	}
	bool IsEmpty(const Piece& piece) const
	{
		return grid[piece.a][piece.b] == EMPTY;
	}
	void MakeMove(const Piece& piece, int player)
	{
		
		grid[piece.a][piece.b] = player;
		history.push_back(piece);
	}
	void undoMove()
	{
		if (history.empty()) return;

		auto last = history.back();
		history.pop_back();
		grid[last.a][last.b] = EMPTY;
	}

	int GetCell(const Piece& piece) const
	{
		return grid[piece.a][piece.b];
	}
	void ReSet()
	{
		for (int i = 0; i < BoardSize; i++)
		{
			for (int j = 0; j < BoardSize; j++)
			{
				grid[i][j] = EMPTY;
			}
		}
		history.clear();
	}
	Piece LastMove() const
	{
		if (history.empty())
			return Piece(-1, -1);
		return history.back();
	}
	bool IsEmptyBoard() const
	{
		return history.empty();
	}
	bool IsFull()const
	{
		for (int i = 0; i < BoardSize; i++)
		{
			for (int j = 0; j < BoardSize; j++)
			{
				if (grid[i][j] == EMPTY)
					return false;
			}
		}
		return true;
	}
};
void PrintBoard(const Board& board)
{
	std::cout << std::endl << "Board Now" << std::endl;
	for (int i = -1; i < BoardSize; i++)
	{
		if (i < 10)
			std::cout << " ";
		if (i == -1)
		{
			std::cout << "   ";
			for (int j = 0; j < BoardSize; j++)
				if (j < 10)
					std::cout << j << "  ";
				else
					std::cout << j << " ";
					
			std::cout << std::endl;
			continue;
		}
		std::cout << i << " ";
		for (int j = 0; j < BoardSize; j++)
		{
			if (board.grid[i][j] == EMPTY)
				std::cout << " . ";
			else if (board.grid[i][j] == BLACK)
				std::cout << " X ";
			else std::cout << " O ";
		}
		std::cout << std::endl;
	}
}



class RuleEngine
{
public:
	static bool IsWin(const Board& board,const Piece& piece)
	{
		int player = board.GetCell(piece);

		for (int d = 0; d < 4; d++)
		{

			int count = 1;
			count += CountDirection(board, piece, dir_piece[d], player);
			count += CountDirection(board, piece, -dir_piece[d],player);
			if (count > 4)
				return true;
		}
		return false;

	}
	static bool IsForbidden(const Board& board, const Piece& piece)
	{
		int player = board.GetCell(piece);
		if (player != BLACK)
			return false;
		if (CountLiveThree(board, piece, player) > 1)
			return true;
		if (CountLiveFour(board, piece, player) > 1)
			return true;
		if (CountDeadFour(board, piece, player) > 1)
			return true;
		if (IsLongThanFive(board, piece, player))
			return true;
		return false;
	}


	static int CountDirection(const Board& board, const Piece& piece, const Piece& dir_piece, int player)
	{
		int cnt = 0;
		Piece next_piece = piece + dir_piece;

		while (board.IsInside(next_piece) && board.GetCell(next_piece) == player)
		{
			cnt++;
			next_piece = next_piece + dir_piece;
		}
		return cnt;
	}
	static std::string GetLineString(const Board& board, const Piece& piece, const Piece& dir_piece)
	{
		std::string line = "";
		for (int i = -4; i < 5; i++)
		{
			Piece i_piece = piece + dir_piece * i;
			if (!board.IsInside(i_piece))
			{
				line+="3";
			}
			else
				line += char("0"+board.GetCell(i_piece));
		}
		return line;
	}
	static int CountPattern(const std::string& line, const std::string& pattern)
	{
		int count = 0;
		for (int i = 0;i+line.size()<pattern.size()+1;i++)
		{
			if (line.substr(i,i + pattern.size()) == pattern)
			{
				count++;
			}
		}
		return count;
	}
	static bool IsLongThanFive(const Board& board, const Piece& piece,int player)
	{

		for(int d = 0; d < 4; d++)
		{
			int count = 1;
			count += CountDirection(board, piece, dir_piece[d], player);
			count += CountDirection(board, piece, -dir_piece[d], player);
			if (count > 5)
				return true;
		}
		return false;
	}
	static int CountLongFive(const Board& board, const Piece& piece, int player)
	{
		int cnt = 0;
		for (int d = 0; d < 4; d++)
		{
			int count = 1;
			count += CountDirection(board, piece, dir_piece[d], player);
			count += CountDirection(board, piece, -dir_piece[d], player);
			if (count > 4)
				cnt = cnt + 1;
		}
		return cnt;
	}

	static int CountLiveFour(const Board& board, const Piece& piece,int player)
	{
		int count = 0;
		for (int d = 0; d < 4; d++)
		{
			std::string line = GetLineString(board, piece, dir_piece[d]);

			if (player == BLACK)
			{
				count += CountPattern(line, "011110");
			}
			else
			{
				count += CountPattern(line, "022220");
			}
		}
		return count;
	}
	static bool IsLiveFour(const Board& board, const Piece& piece,const Piece& dir_piece,int player)
	{
		std::string line = GetLineString(board, piece, dir_piece);
		if (player == BLACK)
		{
			if (CountPattern(line, "011110") > 0)
				return true;
			else
				return false;
		}
		else
		{
			if (CountPattern(line, "022220") > 0)
				return true;
			else
				return false;
		}
	}
	static int CountLiveThree(const Board& board, const Piece& piece,int player)
	{
		int count = 0;
		Board temp = board;
		for (int d = 0; d < 4; d++)
		{
			bool founded = false;
			for (int i = -4; i < 5; i++)
			{
				Piece next_piece = piece + dir_piece[d] * i;
				if (!board.IsInside(next_piece) or !board.IsEmpty(next_piece))
					continue;
				
				temp.MakeMove(next_piece, player);
				if (IsLiveFour(temp, next_piece, dir_piece[d], player))
				{
					founded = true;
					break;
				}
				temp.undoMove();
			}
			if (founded)
				count++;
		}
		return count;
	}
	static int CountLiveTwo(const Board& board,const Piece& piece,int player)
	{
		int count = 0;
		Board temp = board;
		for (int d = 0; d < 4; d++)
		{
			bool founded = false;
			for (int i = -3; i < 4; i++)
			{
				Piece next_piece = piece + dir_piece[d] * i;
				if (!board.IsInside(next_piece) or !board.IsEmpty(next_piece))
					continue;

				temp.MakeMove(next_piece, player);
				if (CountLiveThree(temp, next_piece, player) > 0)
				{
					founded = true;
					break;
				}
				temp.undoMove();
			}
			if (founded)
				count++;
		}
		return count;



	}
	static int CountDeadFour(const Board& board,const Piece& piece,int player)
	{
		int count = 0;
		for (int d = 0; d < 4; d++)
		{
			std::string line = GetLineString(board, piece, dir_piece[d]);

			if (player == BLACK)
			{
				count += CountPattern(line, "311110");
				count += CountPattern(line, "211110");
				count += CountPattern(line, "011112");
				count += CountPattern(line, "011113");

			}
			else
			{
				count += CountPattern(line, "022223");
				count += CountPattern(line, "022221");
				count += CountPattern(line, "122220");
				count += CountPattern(line, "322220");


			}
		}
		return count;
	}
	static int CountDeadThree(const Board& board,const Piece& piece,int player)
	{
		int count = 0;
		for (int d = 0; d < 4; d++)
		{
			std::string line = GetLineString(board, piece, dir_piece[d]);

			if (player == BLACK)
			{
				count += CountPattern(line, "311100");
				count += CountPattern(line, "211100");
				count += CountPattern(line, "001112");
				count += CountPattern(line, "001113");

			}
			else
			{
				count += CountPattern(line, "002223");
				count += CountPattern(line, "002221");
				count += CountPattern(line, "122200");
				count += CountPattern(line, "322200");


			}
		}
		return count;
	}



};

class Evaluator
{
private:
	enum Score { Five = 1000000, LiveFour = 100000, DeadFour = 10000, LiveThree = 8000, DeadThree = 500, LiveTwo = 100 };

public:
	static int Evalute(const Board& board, int player)
	{
		int myScore = EvaluatePlayer(board, player);
		int opScore = EvaluatePlayer(board, 3 - player);

		return myScore - opScore * 1.2;
	}

	static int EvaluateScore(const Board& board, Piece& piece, int player)
	{
		int score = 0;
		score += RuleEngine::CountLongFive(board, piece, player) * Five;
		score += RuleEngine::CountLiveFour(board, piece, player) * LiveFour;
		score += RuleEngine::CountDeadFour(board, piece, player) * DeadFour;
		score += RuleEngine::CountLiveThree(board, piece, player) * LiveThree;
		score += RuleEngine::CountDeadThree(board, piece, player) * DeadThree;
		score += RuleEngine::CountLiveTwo(board, piece, player) * LiveTwo;
		return score;
	}

	static int EvaluatePlayer(const Board& board, int player)
	{
		int score = 0;
		for (int i = 0; i < BoardSize; i++)
		{
			for (int j = 0; j < BoardSize; j++)
			{
				Piece npiece(i, j);
				if (board.GetCell(npiece) == player)
				{
					score += EvaluateScore(board, npiece, player);
				}

			}
		}
		return score;
	}
	static int EvaluatePoint(Board& board, Piece& piece, int player)
	{
		board.MakeMove(piece, player);
		int score = Evalute(board, player);
		board.undoMove();
		return score;
	}
};


class MoveGenerator
{
public:
	static std::vector<Piece> GenerateMoves( Board& board, int player)
	{
		std::vector<Piece> moves;
		bool vis[BoardSize][BoardSize] = { false };
		for (int i = 0; i < BoardSize; i++)
		{
			for (int j = 0; j < BoardSize; j++)
			{
				Piece piece(i, j);
				if (board.GetCell(piece) != EMPTY)
				{
					for (int dx = -2; dx < 3; dx++)
					{
						for (int dy = -2; dy < 3; dy++)
						{
							Piece dpiece(dx, dy);
							Piece npiece = piece + dpiece;
							if (board.IsInside(npiece) && board.IsEmpty(npiece) && !vis[npiece.a][npiece.b])
							{
								if (player == BLACK)
								{
									board.MakeMove(npiece, player);
									if (RuleEngine::IsForbidden(board,npiece))
									{
										board.undoMove();
										vis[npiece.a][npiece.b] = true;
										continue;
									}
									board.undoMove();
								}

								moves.push_back(npiece);
								vis[npiece.a][npiece.b] = true;
							}
						}
					}
				}

			}
		}
		std::sort(moves.begin(), moves.end(), [&](auto& a, auto& b) {
			return Evaluator::EvaluatePoint(board, a, player) > Evaluator::EvaluatePoint(board, b, player);
			});
		if (moves.size() > 10)
			moves.resize(10);
		return moves;
	}

};


class SearchEngine
{
private:
	static int AlphaBeta(Board& board, int depth, int alpha, int beta, int player)
	{
		if (depth == 0)
		{
			return Evaluator::Evalute(board, player);
		}
		auto moves = MoveGenerator::GenerateMoves(board, player);
		for (auto& move : moves)
		{
			board.MakeMove(move, player);
			int score = -AlphaBeta(board, depth - 1, -beta, -alpha, 3 - player);
			board.undoMove();
			if (score > alpha)
			{
				alpha = score;
			}
			if (alpha > beta)
				break;
		}

		return alpha;
	}
public:
	static Piece FindBestMove(Board& board, int player)
	{
		int bestScore = -INF;
		Piece bestmove;
		auto moves = MoveGenerator::GenerateMoves(board, player);

		for (auto& move : moves)
		{
			board.MakeMove(move, player);
			int score = -AlphaBeta(board, 4, -INF, INF, 3 - player);
			board.undoMove();
			if (score > bestScore)
			{
				bestScore = score;
				bestmove = move;
			}
		}
		return bestmove;
	}


};

class ThreatHandler
{
public:
	static std::vector<Piece> GetImmediateThreats(Board& board, int player)
	{
		std::vector<Piece> threats;
		auto moves = MoveGenerator::GenerateMoves(board, player);
		auto opmoves = MoveGenerator::GenerateMoves(board, 3 - player);
		for (auto move : moves)
		{
			board.MakeMove(move, player);
			if (RuleEngine::IsWin(board, move))
			{
				threats.push_back(move);
			}
			board.undoMove();
		}
		for (auto move : opmoves)
		{
			board.MakeMove(move, 3-player);
			if (RuleEngine::IsWin(board, move))
			{
				threats.push_back(move);
			}
			board.undoMove();
		}
		for (auto move : moves)
		{
			board.MakeMove(move, player);
			if (RuleEngine::CountLiveFour(board, move,player) > 0)
			{
				threats.push_back(move);
			}
			board.undoMove();
		}
		return threats;
	}

};

class Play
{
public:
	static int ChooseHumanPlayer()
	{
		int humanplayer;
		std::cout << "Please choose Black or White: 1=Black, 2=White " << std::endl;
		std::cin >> humanplayer;
		if (humanplayer == BLACK)
		{
			std::cout << "You have chosen the Black." << std::endl;
			std::cout << "You need go first." << std::endl;
			return humanplayer;
		}
		else if (humanplayer == WHITE)
		{
			std::cout << "You have chosen the White" << std::endl;
			std::cout << "Please wait AI to make its move." << std::endl;
			return humanplayer;

		}
		else
		{
			std::cout << "You should choose 1(Black) or 2(White).PLease choose again.";
			return ChooseHumanPlayer();
		}

	}
	static Piece HumanMove(Board& board,int humanplayer)
	{
		int row, column;
		std::cout << "Please choose your move position: row column " << std::endl;
		std::cin >> row >> column;
		Piece humanmove(row, column);
		board.MakeMove(humanmove, humanplayer);
		PrintBoard(board);
		std::cout << "You have placed your piece on row " << humanmove.a << ", column " << humanmove.b << std::endl;
		std::cout<<"Please wait for AI to make its move." << std::endl;
		return humanmove;
	}
	static Piece AIMove(Board& board,int humanplayer)
	{
		int aiplayer = 3 - humanplayer;
		Piece aimove;
		std::vector<Piece> Threatmove = ThreatHandler::GetImmediateThreats(board, aiplayer);
		if (Threatmove.size() > 0)
		{
			aimove = Threatmove.front();
			board.MakeMove(aimove, aiplayer);

		}
		else
		{
			aimove = SearchEngine::FindBestMove(board, aiplayer);
			board.MakeMove(aimove, aiplayer);

		}
		PrintBoard(board);
		std::cout << "AI has chosen its move" << std::endl;
		return aimove;
	}
	static bool WIN(const Board& board,Piece& piece)
	{
		int player = board.GetCell(piece);
		if (RuleEngine::IsWin(board, piece))
		{
			if (player == BLACK)
			{
				std::cout << "Player Black has win!" << std::endl;

			}
			else
			{
				std::cout << "Player White has win!" << std::endl;

			}
			PrintBoard(board);
			std::cout << "The final board is here." << std::endl;
			return true;
		}
		return false;



	}
	static void PlayTurn()
	{
		Board board;
		const int humanplayer = Play::ChooseHumanPlayer();
		PrintBoard(board);
		int turn = 0;
		int nowplayer=BLACK;
		if (humanplayer == WHITE)
		{
			int center = (BoardSize / 2) - 1;
			Piece begin(center, center);
			board.MakeMove(begin, BLACK);
			PrintBoard(board);
			std::cout << "AI has chosen its move" << std::endl;
			nowplayer = WHITE;
			turn = turn + 1;
		}



		while (turn <BoardSize*BoardSize+1)
		{
			turn = turn + 1;
			Piece move;
			if (nowplayer == humanplayer)
			{
				move=HumanMove(board, humanplayer);
				
			}
			else
			{
				move = AIMove(board, humanplayer);
			}
			
			if (WIN(board, move))
			{
				break;
			}

			nowplayer = 3 - nowplayer;
		}
		std::cout << "You have played a total of " << turn << " moves with AI." << std::endl;
		std::cout << "If you want play again , please type 1 or type other to end:" << std::endl;
		int again;
		std::cin >> again;
		if (again == 1)
			PlayTurn();
	}
	

};


int main()
{
	Play::PlayTurn();


	std::cin.get();
}