#include <Magick++.h>
#include <vector>
#include <string>
#include <math.h>

using namespace std; 
using namespace Magick;
int main(int argc, char **argv)
{
	for(int numsets = 0; numsets < 20; numsets++)
	{
		InitializeMagick(*argv);
		Image img(argv[1] + to_string(numsets) + ".png");
		int width = img.columns();
		int height = img.rows();
		int numIterations = log2(min(width, height));

		//generate different resolutions of the image
		for( int i = 0; i <= numIterations; i++)
		{
			img.write("output_level_" + to_string(i) + "-" + to_string(numsets) + ".png");
			img.scale("50%");
		}
	}
	return 0;
}
