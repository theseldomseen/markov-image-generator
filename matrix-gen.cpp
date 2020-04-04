#include <fstream>
#include <Magick++.h>
#include <vector>
#include <string>
#include <math.h>
#include <cstdio>
#include <time.h>

using namespace std;
using namespace Magick;

int markov(vector <double> arr, int prev_val);
int main(int argc, char **argv)
{
	/*
	HOW THE PROGRAM WORKS:
	This code takes a number of images, with 12 layers of resolution. 
	There's some helper code that generates these layers, basically each progressive layer is an image downsampled to half resoluton. 
		for example a (6000 * 4000) image would result in 12 layers, (6000 * 4000)
									     (3000 * 2000)
									     (1500 * 1000)
									     (750  *  500)
									     (375  *  250)
									     (188  *  125)
									     (94   *   63)
									     (47   *   32)
									     (24   *   16)
									     (12   *    8)
									     (6    *    4)
									     (3    *    2)

	The code takes these inputs and generates probability matricies out of them. 
	It looks from low resolution to high, going from a single pixel into 4 pixels on the next layer. 
	Each of these four regions has its own unique matrix per color, and each layer has its own set of these four regions. 
	So in total there are three probability matricies (one per color) with dimensions [num layers]*[num regions = 4]*[256]*[256]

	Once these probability matricies are generated and normalized, the code generates output images in layers from low to high. 
	The generation happens the same way the training happens. You start with a very low resolution image and then upscale by splitting each pixel into 4 regions iteratively. 
	Right now the initial 3 x 2 image it starts from is one of the 3 x 2 images from the training set. 
	*/



	//initialize the probability matricies for each color. 
	vector<vector<vector<vector <double> > > > red(10, vector<vector<vector <double> > >(4, vector<vector<double> >(256, vector<double>(256))));
        vector<vector<vector<vector <double> > > > blue(10, vector<vector<vector <double> > >(4, vector<vector<double> >(256, vector<double>(256))));
	vector<vector<vector<vector <double> > > > green(10, vector<vector<vector <double> > >(4, vector<vector<double> >(256, vector<double>(256))));
	
	InitializeMagick(*argv);
	
	for(int numsets = 0; numsets < 20; numsets++) //go through each image set  (in this case I have 20 images that the training set is made from)
	{
		for(int i = 9; i >= 0; i--) //go through each transition layer
		{
			printf("on image %d, iter %d...\n\n", numsets, i);
			Image img_cur("output_level_" + to_string(i+1) + "-" + to_string(numsets) + ".png"); //open the current and next layer, so you can get the transition
			Image img_nxt("output_level_" + to_string(i) + "-" + to_string(numsets) + ".png");
			int width_nxt = img_nxt.rows();
			int height_nxt = img_nxt.columns();
			int width_cur = img_cur.rows();
			int height_cur = img_cur.columns();
			for(int x = 0; x < width_cur; x++) //go through each pixel of the lower resolution layer
			{
				for(int y = 0; y < height_cur; y++)
				{
					ColorRGB pixel_cur = img_cur.pixelColor(x, y); //fetch the RGB color values of the current pixel

					ColorRGB pixel_nxt = img_nxt.pixelColor(x * 2, y * 2); //these four blocks look at each of the four regions in the next layer. top left, top right, bottom left, bottom right
					red[i][0][(int)(pixel_cur.red() * 255)][(int)(pixel_nxt.red() * 255)] += 1; //increment the value in layer i, region 0, from the current layer color to the next layer color
					green[i][0][(int)(pixel_cur.green() * 255)][(int)(pixel_nxt.green() * 255)] += 1; //these all are what're generating the probability matrix for the markov chain
					blue[i][0][(int)(pixel_cur.blue() * 255)][(int)(pixel_nxt.blue() * 255)] += 1;

					pixel_nxt = img_nxt.pixelColor((x * 2) + 1, y * 2); //top right
					red[i][1][(int)(pixel_cur.red() * 255)][(int)(pixel_nxt.red() * 255)] += 1;
					green[i][1][(int)(pixel_cur.green() * 255)][(int)(pixel_nxt.green() * 255)] += 1;
					blue[i][1][(int)(pixel_cur.blue() * 255)][(int)(pixel_nxt.blue() * 255)] += 1;
					
					pixel_nxt = img_nxt.pixelColor(x * 2, (y * 2) + 1); //bottom left
					red[i][2][(int)(pixel_cur.red() * 255)][(int)(pixel_nxt.red() * 255)] += 1;
					green[i][2][(int)(pixel_cur.green() * 255)][(int)(pixel_nxt.green() * 255)] += 1;
					blue[i][2][(int)(pixel_cur.blue() * 255)][(int)(pixel_nxt.blue() * 255)] += 1;
					
					pixel_nxt = img_nxt.pixelColor((x * 2) + 1, (y * 2) + 1); //bottom right
					red[i][3][(int)(pixel_cur.red() * 255)][(int)(pixel_nxt.red() * 255)] += 1;
					green[i][3][(int)(pixel_cur.green() * 255)][(int)(pixel_nxt.green() * 255)] += 1;
					blue[i][3][(int)(pixel_cur.blue() * 255)][(int)(pixel_nxt.blue() * 255)] += 1;
				}
			}
		}
	}

	for(int layer = 9; layer >= 0; layer--) //perform normalization so you get decimal probabilities at each coordinate in the matrix
	{
		for(int a = 0; a < 4; a++)
		{
			for(int b = 0; b < red[layer][a].size(); b++)
			{
				double tot_r = 0;
				double tot_g = 0;
				double tot_b = 0;
				for(int c = 0; c < red[layer][a][b].size(); c++)
				{
					tot_r += red[layer][a][b][c];
					tot_g += green[layer][a][b][c];
					tot_b += blue[layer][a][b][c];
				}
				for(int c = 0; c < red[layer][a][b].size(); c++)
				{
					if (tot_r > 0)
					{
						red[layer][a][b][c] = red[layer][a][b][c] / tot_r; //just simple averaging
					}

					if (tot_g > 0)
					{
						green[layer][a][b][c] = green[layer][a][b][c] / tot_g;
					}

					if (tot_b > 0)
					{
						blue[layer][a][b][c] = blue[layer][a][b][c] / tot_b;
					}
				}
			}
		}
	}

	srand(time(NULL));
	printf("seed: %ld", time(NULL)); //seeding

	double color_r = 0; //
	double color_g = 0;
	double color_b = 0;
	int color_prev_r = 0;
	int color_prev_g = 0;
	int color_prev_b = 0;
	int width = 8;
	int height = 8;
	Image img_prev;
	bool sw = 0;

	for(int layer = 9; layer >= 0; layer--)
	{
		printf("on layer number %d\n width is %d and height is %d\n", layer, width, height);
		Geometry dim(to_string(width) + "x" + to_string(height)); //generate dimensions as a Geometry object
		Image img_out(dim, "black"); //initialize the output image
		for(int x = 0; x < width-1; x++)
		{
			for(int y = 0; y < height-1; y++)
			{
				if (sw) //this is designed to be skipped the first time since there's no previous image yet
				{
					ColorRGB px = img_prev.pixelColor(x / 2, y / 2);
					color_prev_r = (int) (px.red() * 255);
					color_prev_g = (int) (px.green() * 255);
					color_prev_b = (int) (px.blue() * 255);
				}
				else //this opens a specified image as the "previous image" to have a sort of starting seed.
				{
					Image img_in("xyz.png");
					ColorRGB px = img_in.pixelColor(x / 2, y / 2);
                                        color_prev_r = (int) (px.red() * 255);
                                        color_prev_g = (int) (px.green() * 255);
                                        color_prev_b = (int) (px.blue() * 255);
				}
				for(int region = 0; region < 4; region++) //this does all the generation using my markov function which looks at the probabilities given by a row and selects the next index. 
									  //if the row passed is full of zeros, then just hold the previous color. 
				{
					color_r = (((double)markov(red[layer][region][color_prev_r], color_prev_r)) / 256);
					color_g = (((double)markov(green[layer][region][color_prev_g], color_prev_g)) / 256);
					color_b = (((double)markov(blue[layer][region][color_prev_b], color_prev_b)) / 256);
					ColorRGB pixel(color_r, color_g, color_b);
					img_out.pixelColor((x + region % 2), (y + region / 2), pixel);
				}
			}
		}
		img_prev = img_out;
		double thing = (((double)layer)/9.0) + 0.5;
		img_prev.blur(0.5, thing);
		sw = 1;
		img_out.write("generated_output_level_" + to_string(layer) + ".png");
		width *= 2;
		height *= 2;
	}
	return 0;
}

int markov(vector <double> arr, int prev_val)
{
        double n = 0;
        double x = (double)rand() / (double)RAND_MAX; // generate random number in range [0,1]
        for(int i = 0; i < arr.size(); i++)
        {
                n += arr[i]; // add the probability
                if(x <= n) //this'll be x with a probability distribution given in the input array
                {
                        return i;
                }
        }
        return prev_val;
}




