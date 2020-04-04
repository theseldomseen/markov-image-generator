#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <time.h>
using namespace std;
int markov(vector <double> arr);

int main( int argc, char **argv)
{
	srand(time(NULL));
	vector<vector <double> > array{ {0.8 , 0.2 , 0.0 },
					{0.05, 0.6 , 0.35},
					{0.05, 0.45, 0.5 } };
	int xn = 0;
	for(int a = 0; a < 1000; a++)
	{
		xn = markov(array[xn]);
		if (xn == 0)
			printf("sunny\n");
		else if (xn == 1)
			printf("cloudy\n");
		else if (xn == 2)
			printf("rainy\n");
		else
			printf("error, xn = %d\n", xn);
	}
}


int markov(vector <double> arr)
{
	double n = 0;
	double x = (double)rand() / (double)RAND_MAX; // generate random number in range [0,1]
	for(int i = 0; i < arr.size(); i++)
	{
		n += arr[i]; // add the probability
		if(x <= n) //this'll be x with a probability distribution given in te input array
		{
			return i;
		}
	}

	return INT_MAX(); 
}
