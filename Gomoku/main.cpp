#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<random>
#include<unordered_map>
#include <cstdint> 
#include<iterator>

const int BoardSize = 38;
enum Cell { EMPTY = 0, BLACK = 1, WHITE = 2 };
const int INF = 1e9;
class Evaluator;
class MoveGenerator;

struct TTEntry {     //定义置换表
	int depth;
	int value;
};
std::unordered_map<uint64_t, TTEntry> TT;

//哈希，会用异或
class Zobrist 
{
public:
	static uint64_t table[BoardSize][BoardSize][3];
	static uint64_t sideKey;
	static void Init()
	{
		std::cout << "Initializing Zobrist table...\n";
		std::mt19937_64 rng(123456); // 梅森旋转 64 位随机引擎，固定种子

		for (int i = 0; i < BoardSize; i++)
			for (int j = 0; j < BoardSize; j++)
				for (int k = 0; k < 3; k++)
					table[i][j][k] = rng();

		sideKey = rng();
	}


};
uint64_t Zobrist::table[BoardSize][BoardSize][3];
uint64_t Zobrist::sideKey;


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
	uint64_t hashKey = 0;
	std::vector<Piece> history;
	int currentPlayer = BLACK;

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
		if (!IsInside(piece))
			return;
		grid[piece.a][piece.b] = player;
		hashKey ^= Zobrist::table[piece.a][piece.b][player];

		hashKey ^= Zobrist::sideKey;   // 切换回合
		currentPlayer = 3 - player;

		history.push_back(piece);
	}
	void undoMove()
	{
		if (history.empty()) return;

		auto last = history.back();
		history.pop_back();
		int player = grid[last.a][last.b];

		hashKey ^= Zobrist::sideKey;   // 先切回去
		currentPlayer = player;

		hashKey ^= Zobrist::table[last.a][last.b][player];
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

		hashKey = 0;

		currentPlayer = BLACK;        // 默认黑先
		hashKey ^= Zobrist::sideKey;  //表示“黑走”
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

};
void PrintBoard(const Board& board)
{
	std::cout << std::endl << "Board Now" << std::endl;
	for (int i = -1; i < BoardSize+1; i++)
	{
		if (i < 10 || i == BoardSize)
			std::cout << " ";
		if (i == -1 || i == BoardSize)
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
	static bool IsForbidden(Board& board, const Piece& piece)
	{
		int player = board.GetCell(piece);
		if (player != BLACK)
			return false;
		if (CountLiveThree(board, piece, player) > 1)
			return true;
		if (CountLiveFour(board, piece, player) + CountDeadFour(board, piece, player) > 1)
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
				line += char('0'+board.GetCell(i_piece));
		}
		return line;
	}
	static int CountPattern(const std::string& line, const std::string& pattern)
	{
		int count = 0;
		for (int i = 0; i + pattern.size() < line.size() + 1; i++)
		{
			if (line.substr(i, pattern.size()) == pattern)
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
				count += CountPattern(line, "0101110");
				count += CountPattern(line, "0111010");
				count += CountPattern(line, "0110110");


			}
			else
			{
				count += CountPattern(line, "022220");
				count += CountPattern(line, "0202220");
				count += CountPattern(line, "0222020");
				count += CountPattern(line, "0220220");

			}
		}
		return count;
	}
	static bool IsLiveFour(const Board& board, const Piece& piece,int player)
	{
		if (CountLiveFour(board,piece,player)> 0)
			return true;
		else
			return false;
		
	}
	static int CountLiveThree(Board& board, const Piece& piece,int player)
	{
		int count = 0;
		
		for (int d = 0; d < 4; d++)
		{
			bool founded = false;
			for (int i = -4; i < 5; i++)
			{
				Piece next_piece = piece + dir_piece[d] * i;
				if (!board.IsInside(next_piece) or !board.IsEmpty(next_piece))
					continue;
				
				board.MakeMove(next_piece, player);
				if (IsLiveFour(board, next_piece, player))
				{
					board.undoMove();
					founded = true;
					break;
				}
				board.undoMove();
			}
			if (founded)
				count++;
		}
		return count;
	}
	static int CountLiveTwo(Board& board,const Piece& piece,int player)
	{
		int count = 0;
		for (int d = 0; d < 4; d++)
		{
			bool founded = false;
			for (int i = -3; i < 4; i++)
			{
				Piece next_piece = piece + dir_piece[d] * i;
				if (!board.IsInside(next_piece) or !board.IsEmpty(next_piece))
					continue;

				board.MakeMove(next_piece, player);
				if (CountLiveThree(board, next_piece, player) > 0)
				{
					board.undoMove();
					founded = true;
					break;
				}
				board.undoMove();
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


class PreEvaluator
{
public:
	static int EvaluateMove(Board& board, const Piece& p, int player)
	{
		int attack = EvaluatePoint(board, p, player);
		int defense = EvaluatePoint(board, p, 3 - player);

		// 防守更重要（关键）
		return attack + defense * 2;
	}

	/*
	static int EvaluateScore(Board& board, Piece& piece, int player)
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

	static int EvaluatePlayer(Board& board, int player)
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
		}*/

	static int EvaluatePoint(Board& board, const Piece& p, int player)
	{
		int score = 0;

		board.MakeMove(p, player);


		int connect = 1;
		for (int d = 0; d < 4; d++) {
			int cnt = 1;
			cnt += RuleEngine::CountDirection(board, p, dir_piece[d], player);
			cnt += RuleEngine::CountDirection(board, p, -dir_piece[d], player);
			connect = std::max(connect, cnt);
		}
		// 强化连续性（核心！）
		if (connect == 2) score += 5000;
		if (connect == 3) score += 50000;
		if (connect >= 4) score += 500000;


		int liveFour = RuleEngine::CountLiveFour(board, p, player);
		int deadFour = RuleEngine::CountDeadFour(board, p, player);
		int liveThree = RuleEngine::CountLiveThree(board, p, player);
		int deadThree = RuleEngine::CountDeadThree(board, p, player);
		int liveTwo = RuleEngine::CountLiveTwo(board, p, player);

		// 绝杀优先
		if (liveFour > 0) score += 1000000;
		if (deadFour > 1) score += 500000;

		// 组合威胁
		if (liveThree > 1) score += 200000; // 双三

		// 常规评分
		score += liveFour * 100000;
		score += deadFour * 20000;
		score += liveThree * 8000;
		score += deadThree * 1000;
		score += liveTwo * 100;

		//中心偏好
		int center = BoardSize / 2;
		score -= abs(p.a - center) * 10;
		score -= abs(p.b - center) * 10;

		board.undoMove();

		return score;
	}
};


class MoveGenerator
{
public:
	static std::vector<Piece> GenerateMoves(Board& board, int player)
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
									if (RuleEngine::IsForbidden(board, npiece))
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
		/*std::sort(moves.begin(), moves.end(), [&](auto& a, auto& b) {
			return PreEvaluator::EvaluateMove(board, a, player) > PreEvaluator::EvaluateMove(board, b, player);
			})*/
			// 2. 【优化2】计算分数并只保留高价值点（分数 > 500）
		std::vector<std::pair<Piece, int>> scored;
		for (auto& m : moves) {
			int s = PreEvaluator::EvaluateMove(board, m, player);  // 注意这里用 Evaluator（你的类名）
			if (s > 500) {          // 阈值可调
				scored.push_back({ m, s });
			}
		}

		// 如果过滤后还有剩余，就用过滤后的；否则退化为保留原 moves（防止极端情况无棋可走）
		if (!scored.empty()) {
			// 按分数从高到低排序
			std::sort(scored.begin(), scored.end(),
				[](const auto& a, const auto& b) { return a.second > b.second; });

			moves.clear();
			for (auto& x : scored)
				moves.push_back(x.first);
		}
		// 如果 scored 为空，moves 保持未过滤（原样）
		;
		if (moves.size() > 10)
			moves.resize(10);
		/*
		std::copy(moves.begin(), moves.end(), std::ostream_iterator<decltype(moves)::value_type>(std::cout, " "));
		std::cout << std::endl;
		*/

		return moves;
	}

};


class Evaluator
{
private:
	//enum Score { Five = 1000000, LiveFour = 100000, DeadFour = 10000, LiveThree = 8000, DeadThree = 500, LiveTwo = 100 };

public:
	static int Evalute2(Board& board, int player)
	{
		auto myMoves = MoveGenerator::GenerateMoves(board, player);
		auto opMoves = MoveGenerator::GenerateMoves(board, 3 - player);

		int myScore = 0;
		int opScore = 0;

		for (auto& m : myMoves)
		{
			myScore += PreEvaluator::EvaluateMove(board, m, player);
		}

		for (auto& m : opMoves)
		{
			opScore += PreEvaluator::EvaluateMove(board, m, 3 - player);
		}

		return myScore - opScore * 1.1;
	}
	static int Evalute(Board& board, int player)
	{
		auto myMoves = MoveGenerator::GenerateMoves(board, player);
		auto opMoves = MoveGenerator::GenerateMoves(board, 3 - player);

		int myScore = 0;
		int opScore = 0;
		int bestMy = 0;
		for (auto& m : myMoves)
		{
			bestMy = std::max(bestMy, PreEvaluator::EvaluateMove(board, m, player));
		}

		int bestOp = 0;
		for (auto& m : opMoves)
		{
			bestOp = std::max(bestOp, PreEvaluator::EvaluateMove(board, m, 3 - player));
		}

		return bestMy - bestOp * 1.2;
	}
	
};





class SearchEngine
{
private:
	static int AlphaBeta(Board& board, int depth, int alpha, int beta, int player)
	{
		auto it = TT.find(board.hashKey);

		if (it != TT.end() && it->second.depth >= depth)
		{
			return it->second.value;
		}

		if (depth == 0)
		{
			int val = Evaluator::Evalute(board, player);
			TT[board.hashKey] = { depth,val };
			return val;
		}
		auto moves = MoveGenerator::GenerateMoves(board, player);

		int best = -INF;

		for (auto& move : moves)
		{
			board.MakeMove(move, player);
			if (RuleEngine::IsWin(board, move))
			{
				board.undoMove();
				return 10000000;
			}

			int score = -AlphaBeta(board, depth - 1, -beta, -alpha, 3 - player);
			board.undoMove();

			if (score > best)
				best = score;
			if (score > alpha)
				alpha = score;
			if (alpha >= beta)
				break;
		}
		TT[board.hashKey] = { depth,best };

		return best;
	}
public:
	static Piece FindBestMove(Board& board, int player)
	{
		int bestScore = -INF;
		Piece bestmove;
		auto moves = MoveGenerator::GenerateMoves(board, player);
		if (moves.empty())
			return Piece(-1, -1);
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
/*
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
*/

class ThreatHandler
{
public:
	// 找自己能赢的点
	static std::vector<Piece> GetWinningMoves(Board& board, int player)
	{
		std::vector<Piece> res;
		auto moves = MoveGenerator::GenerateMoves(board, player);

		for (auto& m : moves)
		{
			board.MakeMove(m, player);
			if (RuleEngine::IsWin(board, m))
				res.push_back(m);
			board.undoMove();
		}
		return res;
	}

	// 找对方要赢的点（必须堵）
	static std::vector<Piece> GetBlockingMoves(Board& board, int player)
	{
		int opponent = 3 - player;
		std::vector<Piece> res;

		auto moves = MoveGenerator::GenerateMoves(board, opponent);

		for (auto& m : moves)
		{
			board.MakeMove(m, opponent);
			if (RuleEngine::IsWin(board, m))
				res.push_back(m);
			board.undoMove();
		}
		return res;
	}

	// 活四（进攻）
	static std::vector<Piece> GetLiveFourMoves(Board& board, int player)
	{
		std::vector<Piece> res;
		auto moves = MoveGenerator::GenerateMoves(board, player);

		for (auto& m : moves)
		{
			board.MakeMove(m, player);
			if (RuleEngine::CountLiveFour(board, m, player) > 0)
				res.push_back(m);
			board.undoMove();
		}
		return res;
	}

	// 对方活四（必须防）
	static std::vector<Piece> GetOpponentLiveFour(Board& board, int player)
	{
		int opponent = 3 - player;
		std::vector<Piece> res;

		auto moves = MoveGenerator::GenerateMoves(board, opponent);

		for (auto& m : moves)
		{
			board.MakeMove(m, opponent);
			if (RuleEngine::CountLiveFour(board, m, opponent) > 0)
				res.push_back(m);
			board.undoMove();
		}
		return res;
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

	static Piece AIMove(Board& board, int humanplayer)
	{
		int ai = 3 - humanplayer;

		// 自己直接赢
		auto winMoves = ThreatHandler::GetWinningMoves(board, ai);
		if (!winMoves.empty())
		{
			board.MakeMove(winMoves[0], ai);
			PrintBoard(board);
			std::cout << "AI has chosen its move" << std::endl;
			return winMoves[0];
		}

		// 防对方直接赢
		auto blockMoves = ThreatHandler::GetBlockingMoves(board, ai);
		if (!blockMoves.empty())
		{
			board.MakeMove(blockMoves[0], ai);
			PrintBoard(board);
			std::cout << "AI has chosen its move" << std::endl;
			return blockMoves[0];
		}

		// 自己活四
		auto liveFour = ThreatHandler::GetLiveFourMoves(board, ai);
		if (!liveFour.empty())
		{
			board.MakeMove(liveFour[0], ai);
			PrintBoard(board);
			std::cout << "AI has chosen its move" << std::endl;
			return liveFour[0];
		}

		// 防对方活四
		auto opLiveFour = ThreatHandler::GetOpponentLiveFour(board, ai);
		if (!opLiveFour.empty())
		{
			board.MakeMove(opLiveFour[0], ai);
			PrintBoard(board);
			std::cout << "AI has chosen its move" << std::endl;
			return opLiveFour[0];
		}

		// 正常搜索
		Piece best = SearchEngine::FindBestMove(board, ai);
		if (best.a == -1 && best.b == -1)
		{
			std::cout << "AI has no legal moves, game over?" << std::endl;
			return best;
		}
		board.MakeMove(best, ai);
		PrintBoard(board);
		std::cout << "AI has chosen its move" << std::endl;
		return best;
	}
	/*
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
	*/
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
		TT.clear();

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
				std::cout << move << std::endl;
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
	Zobrist::Init();
	Play::PlayTurn();


	std::cin.get();
}