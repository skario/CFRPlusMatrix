#include <limits>
#include <chrono>
#include <random>
#include <vector>
#include <assert.h>
#include "cmdline.h"

using namespace std;
using namespace std::chrono;

const int AlgorithmCount = 3;
char const *algorithmNames[] = { "Fictitious play", "CFR", "CFR+" };
char const *wmodeNames[] = { "constant", "linear", "quadratic" };


class MatrixGame
{
public:
	MatrixGame(int size, mt19937 &rng)
	{
		this->size = size;
		iterationCount = 0;
		payoffs.resize(size * size);

		for (int p = 0; p < 2; p++)
		{
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

		/* rock-paper-scissors
		assert(size == 3);
		payoffs[0 * size + 0] = 0; payoffs[0 * size + 1] = 1; payoffs[0 * size + 2] = -1;
		payoffs[1 * size + 0] = -1; payoffs[1 * size + 1] = 0; payoffs[1 * size + 2] = 1;
		payoffs[2 * size + 0] = 1; payoffs[2 * size + 1] = -1; payoffs[2 * size + 2] = 0;
		*/
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

	void CFRPlus(int delay, int wmode)
	{
		iterationCount++;
		CFRPlus(0, delay, wmode);
		CFRPlus(1, delay, wmode);
	}

	void Iteration(int algorithm, int delay, int wmode)
	{
		switch (algorithm)
		{
		case 0: FictitiousPlay(); break;
		case 1: CFR(); break;
		default: CFRPlus(delay, wmode); break;
		}
	}

	void Dump()
	{
		printf("Payoffs:\n");

		for (int a = 0; a < size; a++)
		{
			for (int b = 0; b < size; b++)
			{
				printf("%5.2f ", payoffs[a * size + b]);
			}
			printf("\n");
		}

		for (int p = 0; p < 2; p++)
		{
			auto ns = GetNormalizedStrategy(p);

			printf(p == 0 ? "Row strategy:    " : "Column strategy: ");
			
			for (int i = 0; i < size; i++)
				printf("%4.2f ", ns[i]);

			printf("\n");
		}
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

	void CFRPlus(int player, int delay, int wmode)
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

		double w;

		switch(wmode)
		{
		case 0: w = 1; break;
		case 1: w = iterationCount > delay ? (iterationCount - delay) : 0; break;
		default: w = iterationCount > delay ? (iterationCount - delay) * (iterationCount - delay) : 0; break;
		}

		for (int a = 0; a < size; a++)
			strategy[player][a] += sp[a] * w;

	}


private:
	int size;
	int iterationCount;
	vector<double> payoffs;
	vector<double> strategy[2];
	vector<double> cfr[2];

};

int Run(int algorithm, int delay, int wmode, int size, double epsilon, mt19937 &rng)
{
	MatrixGame m(size, rng);
	double e;
	do
	{
		m.Iteration(algorithm, delay, wmode);
		e = m.GetExploitability();
	} while (e > epsilon);

	return m.GetIterationCount();
}

void RunMany(int n, int algorithm, int delay, int wmode, int size, double epsilon)
{
	random_device rd;
	mt19937 rng;
	rng.seed(rd());

	double sum = 0;
	int min = numeric_limits<int>::max();
	int max = numeric_limits<int>::min();

	for (int i = 0; i < n; i++)
	{
		printf("\r%d/%d", i + 1, n);
		fflush(stdout);
		auto nit = Run(algorithm, delay, wmode, size, epsilon, rng);
		min = std::min(min, nit);
		max = std::max(max, nit);
		sum += nit;
	}

	printf("\r%-16s | min %-4d | max %-6d | avg %.1f\n", algorithmNames[algorithm], min, max, sum / n);

}

int main(int argc, char *argv[])
{
	CommandLine::Integer algorithm("a", false, "Algorithm (0 = Fictitious play, 1 = CFR, 2 = CFR+)", 0, 2, 2);
	CommandLine::Integer size("s", false, "Matrix size", 2, 100000, 1000);
	CommandLine::Real epsilon("e", false, "Epsilon", 0.000000000001, 1, 0.0001);
	CommandLine::Integer nruns("n", false, "Number of times to run", 1, 100000, 1);
	CommandLine::Boolean all("all", false, "Run all algorithms (used together with -n)");
	CommandLine::Boolean dump("dump", false, "Print payoffs and strategies");
	CommandLine::Integer delay("delay", false, "Averaging delay in iterations", 0, 100000, 0);
	CommandLine::Integer wmode("w", false, "Weighting mode (0 = constant, 1 = linear, 2 = quadratic)", 0, 2, 1);
	CommandLine::Parser::Parse(argc, argv);

	if (!all) printf("Algorithm: %s\n", algorithmNames[algorithm]);
	printf("Averaging delay (CFR+): %d\n", (int)delay);
	printf("Weighting mode (CFR+): %s\n", wmodeNames[(int)wmode]);
	printf("Matrix size: %dx%d\n", (int)size, (int)size);
	printf("Epsilon: %f\n", (double)epsilon);
	printf("N: %d\n", (int)nruns);

	if (nruns > 1)
	{
		if (all)
		{
			for (int alg = 0; alg < AlgorithmCount; alg++)
				RunMany(nruns, alg, delay, wmode, size, epsilon);
		}
		else
		{
			RunMany(nruns, algorithm, delay, wmode, size, epsilon);
		}
			
		return 0;
	}

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
		m.Iteration(algorithm, delay, wmode);

		e = m.GetExploitability();

		auto t = duration_cast<milliseconds>(clock.now() - startTime).count();

		printf("i=%d t=%.2f e=%.6f\n", m.GetIterationCount(), t / 1000.0, e);
	} while (e > epsilon);

	if (dump) m.Dump();

	return 0;
}

