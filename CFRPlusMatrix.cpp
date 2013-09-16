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
char const *distributionNames[] = { "uniform", "normal", "cauchy" };

class MatrixGame
{
public:
	struct Parameters
	{
		int size;
		int distribution;
		int algorithm;
		int delay;
		int wmode;
	};

	MatrixGame(Parameters const &p, mt19937 &rng)
		: p(p)
	{
		iterationCount = 0;
		payoffs.resize(p.size * p.size);

		for (int player = 0; player < 2; player++)
		{
			strategy[player].resize(p.size);
			cfr[player].resize(p.size);
		}

		CreateRandom(rng);
	}

	int GetIterationCount() const { return iterationCount; }
	double GetExploitability() const { return (BestResponse(0) + BestResponse(1)) / 2; }

	void Iteration()
	{
		iterationCount++;

		switch (p.algorithm)
		{
		case 0: FictitiousPlay(); break;
		case 1: CFR(); break;
		default: CFRPlus(); break;
		}
	}

	void Dump()
	{
		printf("Payoffs:\n");

		for (int a = 0; a < p.size; a++)
		{
			for (int b = 0; b < p.size; b++)
			{
				printf("%5.2f ", payoffs[a * p.size + b]);
			}
			printf("\n");
		}

		for (int player = 0; player < 2; player++)
		{
			auto ns = GetNormalizedStrategy(player);

			printf(player == 0 ? "Row strategy:    " : "Column strategy: ");
			
			for (int i = 0; i < p.size; i++)
				printf("%4.2f ", ns[i]);

			printf("\n");
		}
	}

private:
	void CreateRandom(mt19937 &rng)
	{
		switch(p.distribution)
		{
		case 0:
			CreateRandom(rng, uniform_real_distribution<double>(-1.0, 1.0));
			break;
		case 1:
			CreateRandom(rng, normal_distribution<double>(0, 0.5));
			break;
		default:
			CreateRandom(rng, cauchy_distribution<double>(0, 0.02));
			break;
		}

	}

	template <class T>
	void CreateRandom(mt19937 &rng, T distribution)
	{
		for (int a = 0; a < p.size; a++)
		{
			for (int b = 0; b < p.size; b++)
			{
				payoffs[a * p.size + b] = distribution(rng);
			}
		}

		/* rock-paper-scissors
		assert(size == 3);
		payoffs[0 * size + 0] = 0; payoffs[0 * size + 1] = 1; payoffs[0 * size + 2] = -1;
		payoffs[1 * size + 0] = -1; payoffs[1 * size + 1] = 0; payoffs[1 * size + 2] = 1;
		payoffs[2 * size + 0] = 1; payoffs[2 * size + 1] = -1; payoffs[2 * size + 2] = 0;
		*/
	}

	double GetPayoff(int player, int a, int b) const
	{ 
		if (player == 0)
			return payoffs[a * p.size + b]; 
		else
			return -payoffs[b * p.size + a]; 
	}

	vector<double> GetNormalizedStrategy(int player) const
	{
		vector<double> ns(p.size);
		double sum = 0;

		for (int i = 0; i < p.size; i++)
			sum += strategy[player][i];

		if (sum > 0)
		{
			for (int i = 0; i < p.size; i++)
				ns[i] = strategy[player][i] / sum;
		}
		else
		{
			for (int i = 0; i < p.size; i++)
				ns[i] = 1.0 / p.size;
		}

		return ns;
	}

	vector<double> GetCurrentStrategy(int player) const
	{
		vector<double> cs(p.size);
		double sum = 0;

		for (int i = 0; i < p.size; i++)
			sum += max(0.0, cfr[player][i]);

		if (sum > 0)
		{
			for (int i = 0; i < p.size; i++)
				cs[i] = cfr[player][i] > 0 ? cfr[player][i] / sum : 0.0;
		}
		else
		{
			for (int i = 0; i < p.size; i++)
				cs[i] = 1.0 / p.size;
		}

		return cs;
	}

	double BestResponse(int player) const
	{
		double maxSum = -numeric_limits<double>::max();

		auto ns = GetNormalizedStrategy(player ^ 1);

		for (int a = 0; a < p.size; a++)
		{
			double sum = 0;

			for (int b = 0; b < p.size; b++)
				sum += ns[b] * GetPayoff(player, a, b);

			if (sum > maxSum)
				maxSum = sum;
		}

		return maxSum;
	}

	void FictitiousPlay()
	{
		FictitiousPlay(0);
		FictitiousPlay(1);
	}

	void FictitiousPlay(int player)
	{
		double maxSum = -numeric_limits<double>::max();
		int bestAction = -1;

		auto ns = GetNormalizedStrategy(player ^ 1);

		for (int a = 0; a < p.size; a++)
		{
			double sum = 0;

			for (int b = 0; b < p.size; b++)
				sum += ns[b] * GetPayoff(player, a, b);

			if (sum > maxSum)
			{
				maxSum = sum;
				bestAction = a;
			}
		}

		strategy[player][bestAction]++;

	}

	void CFR()
	{
		CFR(0);
		CFR(1);
	}

	void CFR(int player)
	{
		vector<double> cfu(p.size);
		double ev = 0;

		auto sp = GetCurrentStrategy(player);
		auto so = GetCurrentStrategy(player ^ 1);

		for (int a = 0; a < p.size; a++)
		{
			cfu[a] = 0;

			for (int b = 0; b < p.size; b++)
				cfu[a] += so[b] * GetPayoff(player, a, b);

			ev += sp[a] * cfu[a];
		}

		for (int a = 0; a < p.size; a++)
			cfr[player][a] += cfu[a] - ev;

		for (int a = 0; a < p.size; a++)
			strategy[player][a] += sp[a];

	}

	void CFRPlus()
	{
		CFRPlus(0);
		CFRPlus(1);
	}

	void CFRPlus(int player)
	{
		vector<double> cfu(p.size);
		double ev = 0;

		auto sp = GetCurrentStrategy(player);
		auto so = GetCurrentStrategy(player ^ 1);

		for (int a = 0; a < p.size; a++)
		{
			cfu[a] = 0;

			for (int b = 0; b < p.size; b++)
				cfu[a] += so[b] * GetPayoff(player, a, b);

			ev += sp[a] * cfu[a];
		}

		for (int a = 0; a < p.size; a++)
			cfr[player][a] = std::max(0.0, cfr[player][a] + cfu[a] - ev);

		double w;

		if (iterationCount > p.delay)
		{
			int t = iterationCount - p.delay;

			switch(p.wmode)
			{
			case 0: w = 1; break;
			case 1: w = t; break;
			default: w = t * t; break;
			}
		}
		else
		{
			w = 0;
		}

		for (int a = 0; a < p.size; a++)
			strategy[player][a] += sp[a] * w;

	}


private:
	Parameters p;
	int iterationCount;
	vector<double> payoffs;
	vector<double> strategy[2];
	vector<double> cfr[2];

};

int Run(MatrixGame::Parameters const &p, double epsilon, mt19937 &rng)
{
	MatrixGame m(p, rng);
	double e;
	do
	{
		m.Iteration();
		e = m.GetExploitability();
	} while (e > epsilon);

	return m.GetIterationCount();
}

void RunMany(MatrixGame::Parameters const &p, int n, double epsilon, mt19937 &rng)
{
	double sum = 0;
	int min = numeric_limits<int>::max();
	int max = numeric_limits<int>::min();

	for (int i = 0; i < n; i++)
	{
		printf("\r%d/%d", i + 1, n);
		fflush(stdout);
		auto nit = Run(p, epsilon, rng);
		min = std::min(min, nit);
		max = std::max(max, nit);
		sum += nit;
	}

	printf("\r%-16s | min %-4d | max %-6d | avg %.1f\n", algorithmNames[p.algorithm], min, max, sum / n);

}

int main(int argc, char *argv[])
{
	CommandLine::Integer algorithm("a", false, "Algorithm (0 = Fictitious play, 1 = CFR, 2 = CFR+)", 0, 2, 2);
	CommandLine::Integer size("s", false, "Matrix size", 2, 100000, 1000);
	CommandLine::Integer distribution("distribution", false, "Random number distribution (0 = uniform, 1 = normal, 2 = cauchy)", 0, 2, 0);
	CommandLine::Real epsilon("e", false, "Epsilon", 0.000000000001, 1, 0.0001);
	CommandLine::Integer nruns("n", false, "Number of times to run", 1, 100000, 1);
	CommandLine::Boolean all("all", false, "Run all algorithms (used together with -n)");
	CommandLine::Boolean dump("dump", false, "Print payoffs and strategies");
	CommandLine::Integer delay("delay", false, "Averaging delay in iterations", 0, 100000, 0);
	CommandLine::Integer wmode("w", false, "Weighting mode (0 = constant, 1 = linear, 2 = quadratic)", 0, 2, 1);
	CommandLine::Parser::Parse(argc, argv);

	MatrixGame::Parameters p;
	p.size = size;
	p.distribution = distribution;
	p.algorithm = algorithm;
	p.delay = delay;
	p.wmode = wmode;

	if (!all) printf("Algorithm: %s\n", algorithmNames[algorithm]);
	printf("Averaging delay (CFR+): %d\n", (int)delay);
	printf("Weighting mode (CFR+): %s\n", wmodeNames[(int)wmode]);
	printf("Matrix size: %dx%d\n", (int)size, (int)size);
	printf("Random number distribution: %s\n", distributionNames[distribution]);
	printf("Epsilon: %f\n", (double)epsilon);
	printf("N: %d\n", (int)nruns);

	random_device rd;
	mt19937 rng;
	rng.seed(rd());

	if (nruns > 1)
	{
		if (all)
		{
			for (p.algorithm = 0; p.algorithm < AlgorithmCount; p.algorithm++)
				RunMany(p, nruns, epsilon, rng);
		}
		else
		{
			RunMany(p, nruns, epsilon, rng);
		}
			
		return 0;
	}

	printf("init\n");

	MatrixGame m(p, rng);
	double e;

	printf("start\n");

	high_resolution_clock clock;
	auto startTime = clock.now();

	do
	{
		m.Iteration();
		e = m.GetExploitability();
		auto t = duration_cast<milliseconds>(clock.now() - startTime).count();
		printf("i=%d t=%.2f e=%.6f\n", m.GetIterationCount(), t / 1000.0, e);

	} while (e > epsilon);

	if (dump) m.Dump();

	return 0;
}

