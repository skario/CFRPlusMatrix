#include <limits>
#include <chrono>
#include <random>
#include <vector>
#include "cmdline.h"

using namespace std;
using namespace std::chrono;

class MatrixGame
{
public:
	MatrixGame(int size, mt19937 &rng)
	{
		this->size = size;
		iterationCount = 0;

		for (int p = 0; p < 2; p++)
		{
			payoffs.resize(size * size);
			strategy[p].resize(size);
			cfr[p].resize(size);
		}

		CreateRandom(rng);
	}

	void CreateRandom(mt19937 &rng)
	{
		uniform_real_distribution<double> double_rnd(-1.0, 1.0);

		for (int a = 0; a < size; a++)
		{
			for (int b = 0; b < size; b++)
			{
				payoffs[a * size + b] = double_rnd(rng);
			}
		}
	}

	int GetIterationCount() const { return iterationCount; }
	double GetExploitability() const { return (BestResponse(0) + BestResponse(1)) / 2; }

	void FictitiousPlay()
	{
		iterationCount++;
		FictitiousPlay(0);
		FictitiousPlay(1);
	}

	void CFR()
	{
		iterationCount++;
		CFR(0);
		CFR(1);
	}

	void CFRPlus()
	{
		iterationCount++;
		CFRPlus(0);
		CFRPlus(1);
	}

private:
	double GetPayoff(int p, int a, int b) const
	{ 
		if (p == 0)
			return payoffs[a * size + b]; 
		else
			return -payoffs[b * size + a]; 
	}

	vector<double> GetNormalizedStrategy(int player) const
	{
		vector<double> ns(size);
		double sum = 0;

		for (int i = 0; i < size; i++)
			sum += strategy[player][i];

		if (sum > 0)
		{
			for (int i = 0; i < size; i++)
				ns[i] = strategy[player][i] / sum;
		}
		else
		{
			for (int i = 0; i < size; i++)
				ns[i] = 1.0 / size;
		}

		return ns;
	}

	vector<double> GetCurrentStrategy(int player) const
	{
		vector<double> cs(size);
		double sum = 0;

		for (int i = 0; i < size; i++)
			sum += max(0.0, cfr[player][i]);

		if (sum > 0)
		{
			for (int i = 0; i < size; i++)
				cs[i] = cfr[player][i] > 0 ? cfr[player][i] / sum : 0.0;
		}
		else
		{
			for (int i = 0; i < size; i++)
				cs[i] = 1.0 / size;
		}

		return cs;
	}

	double BestResponse(int player) const
	{
		double maxSum = -numeric_limits<double>::max();

		auto ns = GetNormalizedStrategy(player ^ 1);

		for (int a = 0; a < size; a++)
		{
			double sum = 0;

			for (int b = 0; b < size; b++)
				sum += ns[b] * GetPayoff(player, a, b);

			if (sum > maxSum)
				maxSum = sum;
		}

		return maxSum;
	}

	void FictitiousPlay(int player)
	{
		double maxSum = -numeric_limits<double>::max();
		int bestAction = -1;

		auto ns = GetNormalizedStrategy(player ^ 1);

		for (int a = 0; a < size; a++)
		{
			double sum = 0;

			for (int b = 0; b < size; b++)
				sum += ns[b] * GetPayoff(player, a, b);

			if (sum > maxSum)
			{
				maxSum = sum;
				bestAction = a;
			}
		}

		strategy[player][bestAction]++;

	}

	void CFR(int player)
	{
		vector<double> cfu(size);
		double ev = 0;

		auto sp = GetCurrentStrategy(player);
		auto so = GetCurrentStrategy(player ^ 1);

		for (int a = 0; a < size; a++)
		{
			cfu[a] = 0;

			for (int b = 0; b < size; b++)
				cfu[a] += so[b] * GetPayoff(player, a, b);

			ev += sp[a] * cfu[a];
		}

		for (int a = 0; a < size; a++)
			cfr[player][a] += cfu[a] - ev;

		for (int a = 0; a < size; a++)
			strategy[player][a] += sp[a];

	}

	void CFRPlus(int player)
	{
		vector<double> cfu(size);
		double ev = 0;

		auto sp = GetCurrentStrategy(player);
		auto so = GetCurrentStrategy(player ^ 1);

		for (int a = 0; a < size; a++)
		{
			cfu[a] = 0;

			for (int b = 0; b < size; b++)
				cfu[a] += so[b] * GetPayoff(player, a, b);

			ev += sp[a] * cfu[a];
		}

		for (int a = 0; a < size; a++)
			cfr[player][a] = std::max(0.0, cfr[player][a] + cfu[a] - ev);

		for (int a = 0; a < size; a++)
			strategy[player][a] += sp[a] * iterationCount * iterationCount;

	}


private:
	int size;
	int iterationCount;
	vector<double> payoffs;
	vector<double> strategy[2];
	vector<double> cfr[2];

};

char const *algorithmNames[] = { "Fictitious play", "CFR", "CFR+" };

int main(int argc, char *argv[])
{
	CommandLine::Integer algorithm("a", false, "Algorithm (0 = Fictitious play, 1 = CFR, 2 = CFR+)", 0, 2, 2);
	CommandLine::Integer size("s", false, "Matrix size", 2, 100000, 1000);
	CommandLine::Real epsilon("e", false, "Epsilon", 0.000000000001, 1, 0.0001);
	CommandLine::Parser::Parse(argc, argv);

	printf("Algorithm: %s\n", algorithmNames[algorithm]);
	printf("Matrix size: %d\n", (int)size);
	printf("Epsilon: %f\n", (double)epsilon);

	printf("init\n");

	random_device rd;
	mt19937 rng;
	rng.seed(rd());

	MatrixGame m(size, rng);
	double e;

	printf("start\n");

	high_resolution_clock clock;
	auto startTime = clock.now();

	do
	{
		switch (algorithm)
		{
		case 0: m.FictitiousPlay(); break;
		case 1: m.CFR(); break;
		default: m.CFRPlus(); break;
		}

		e = m.GetExploitability();

		auto t = duration_cast<milliseconds>(clock.now() - startTime).count();

		printf("i=%d t=%.2f e=%.6f\n", m.GetIterationCount(), t / 1000.0, e);
	} while (e > epsilon);

	return 0;
}

