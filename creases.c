#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif 
/* not SINGLE */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "generate_creases.h"
#include "triangle.h"

void write_poly(int n, int xrange, int yrange, char* filename){
	srand(time(0)); 
	//print a bunch of things
	FILE *fp;
	fp = fopen(filename, "w+");
	fprintf(fp, "%d %d %d %d\n", n+4, 2, 0, 0);
	for(int i=1;i<=n;i++){
		fprintf(fp, "%d, %d, %d\n", i, rand()%xrange, rand()%yrange);
	}
	fprintf(fp, "%d, %d, %d\n", n+1, 0, 0);
	fprintf(fp, "%d, %d, %d\n", n+2, xrange, 0);
	fprintf(fp, "%d, %d, %d\n", n+3, xrange, yrange);
	fprintf(fp, "%d, %d, %d\n", n+4, 0, yrange);
	fputs("0 0\n",fp);
	fputs("0\n",fp);
}

struct triangulateio *init_in(int n){
	//Initialize a crease pattern with n points. 
	//4 corners of square, and the others randomly distributed inside. 
	if(n<4){
		printf("too few\n");
		return NULL;
	}
	struct triangulateio *in;
	srand(time(0)); 

	in = (struct triangulateio*) malloc(sizeof(struct triangulateio));

	in->numberofpoints = n;
	in->numberofpointattributes = 0;
	in->numberofholes = 0;
	in->numberofregions = 0;
	in->numberofsegments = 0;
	in->numberoftriangles = 0;

	in->pointlist = (REAL*) malloc(in->numberofpoints*2*sizeof(REAL));
	in->pointattributelist = NULL;
	in->pointmarkerlist = NULL;
	in->segmentmarkerlist = NULL;
	in->segmentlist = NULL;
	in->holelist = NULL;
	in->regionlist = NULL;

	in->pointlist[0] = 0;
	in->pointlist[1] = 0;
	in->pointlist[2] = 0;
	in->pointlist[3] = 100;
	in->pointlist[4] = 100;
	in->pointlist[5] = 100;
	in->pointlist[6] = 100;
	in->pointlist[7] = 0;
	for(int i=8;i<2*n;i++){
		in->pointlist[i] = rand()%100;
	}
	return in;
}

struct triangulateio *init_out(struct triangulateio *in){
	//intialize the out datastructure, containing voronoi points calculated by triangle
	struct triangulateio *out;
	out = (struct triangulateio*) malloc(sizeof(struct triangulateio));
	out->pointlist = in->pointlist;
	out->pointmarkerlist = NULL;
	out->trianglelist = NULL;
	out->segmentlist = NULL;
	out->segmentmarkerlist = NULL;
	out->edgelist = NULL;
	out->edgemarkerlist = NULL;
	return out;
}

int **edgelist_to_matrix(struct triangulateio *t){
	//create an undirected matrix, array** based on a list of edges
	int edges = t->numberofedges;
	int points = t->numberofpoints;
	int **matrix = (int**) calloc(points,sizeof(int*));
	for(int i=0;i<points;i++){
		matrix[i] = (int*) calloc(points,sizeof(int));
	}
	for(int i=0;i<2*edges;i+=2){
		int ind1 = t->edgelist[i];
		int ind2 = t->edgelist[i+1];
		matrix[ind1][ind2]=1;
		matrix[ind2][ind1]=1;
	}
	return matrix;
}

void replace_edgelist_using_matrix(struct triangulateio *t, int **matrix){
	int points = t->numberofpoints;
	int *edgelist = (int*) malloc(2*t->numberofedges*sizeof(int));
	int cur = 0;
	for(int i=0;i<points;i++){
		for(int j=i;j<points;j++){
			if(matrix[i][j]==1){
				edgelist[cur*2]=i;
				edgelist[cur*2+1]=j;
				cur++;
			}
		}
	}
	t->edgelist = edgelist;
	t->numberofedges = cur;
}
void free_t(struct triangulateio *t){
	free(t->pointlist);
	free(t->pointattributelist);
	free(t->pointmarkerlist);
	free(t->regionlist);
	free(t->trianglelist);
	free(t->segmentlist);
	free(t->segmentmarkerlist);
	free(t->edgelist);
	free(t->edgemarkerlist);
}

void evenize(int **matrix, int n){
/*
removes 1's so that the number of 1's in each row/column is even
I think this is O(n^2)
*/
//I think this is the thing that doesn't work so well...
//Doesn't always work

	int sum;
	int evens[n];
	for(int i=0;i<n;i++){
		sum = 0;
		for(int j=0;j<n;j++){
			sum+=matrix[i][j];
		}
		if(sum%2==0){
			evens[i]=1;
		}
		else{
			for(int p=i;p<n;p++){
				if(matrix[i][p]==1 && evens[p]!=1) {
					matrix[i][p]=0;
					matrix[p][i]=0;
					evens[p]=1;
					break;
				}
				if(p==n-1){
					printf("got to end oops\n");
				}
			}
			evens[i]=1;
		}
	}

}

REAL cross(REAL x1, REAL y1, REAL x2, REAL y2){
	//cross product
	return (x1*y2 - x2*y1);
}

void swap(int *dawna, int ind1, int ind2){
	int temp = dawna[ind1];
	dawna[ind1] = dawna[ind2];
	dawna[ind2] = temp;
}

int clockwiseof(int ind1, int ind2, int center_index, REAL *pointlist){
	//this takes three points: p1=(x1,y1), p2=(x2,y2), and a center point
	//returns the cross product of the vectors from center to p1 and then p2
	//indicating whether p2 is clockwise from p1 or not
	REAL x1 = pointlist[2*ind1] - pointlist[2*center_index];
	REAL y1 = pointlist[2*ind1+1] - pointlist[2*center_index+1];
	REAL x2 = pointlist[2*ind2] - pointlist[2*center_index];
	REAL y2 = pointlist[2*ind2+1] - pointlist[2*center_index+1];

	printf("%f, %f, %f, %f\n",x1,y1,x2,y2);
    int crossprod =  cross(x1,y1,x2,y2);
	return crossprod;
}

int partitionq(int *indices, REAL *pointlist, int center_index, int start, int end){
	//clockwise quicksort
	srand(time(0));
	int p = start+(rand()%(end-start));
	swap(indices, start, p);
	int i = start;
	int j = end;

	while(i<j){
		// printf("partitioning\n");
		for(int a=start;a<=end;a++){
			printf("%d,",indices[a]);
		}
		printf("\n");
		printf("partition %d\n",p);
		printf("%d\n",clockwiseof(j, p, center_index, pointlist));
		printf("%d\n",clockwiseof(i, p, center_index, pointlist));
		while(clockwiseof(j, p, center_index, pointlist)>0.0 && i<j){
			j--;
		}
		while(clockwiseof(i, p, center_index, pointlist)<=0.0 && i<j){
			i++;
		}
		if(i<j){
			printf("%d,%d\n",i,j);
			swap(indices, i,j);
		}
		else{
			break;
		}
	}
	swap(indices, start, i);
	return i;
}
void clockwisesort(int *indices, REAL *pointlist, int center_index, int start, int end){
	//clockwise quicksort. Not used since I just used the optimized qsort from library
	//I think it had to do with being much easier with how I arranged the data structures
	if(start<end){
		int p = partitionq(indices, pointlist, center_index, start, end);
		clockwisesort(indices, pointlist, center_index, start, p-1);
		clockwisesort(indices, pointlist, center_index, p+1 ,end);
	}
}

void clockwiseSelect(int *indices, REAL *pointlist, int center_index, int start, int end){
	//select sort clockwise?
	for(int i=start;i>end;i++){
		int j = i;
		while(j>start && clockwiseof(indices[j], indices[j-1], center_index, pointlist)>0.0){
			swap(indices, j, j-1);
			j--;
		}
	}
}

void drawstring(int x, int y, int z, const char *s)   {
	//use opengl draw a word
	int i;
	for (i = 0; i < strlen(s); i++)
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
}
void printrows(int **dawna){
	for(int i=0;i<14;i++){
		for(int j=0;j<14;j++){
			printf("%d ",dawna[i][j]);
		}
		printf("\n");
	}
}

void printevens(int **dawna){
	//NEEDS TO GENERALIZE PAST 14
	//though I don't think this is used anywhere
	for(int i=0;i<14;i++){
		int sum = 0 ;
		for(int j=0;j<14;j++){
			if(dawna[i][j]==1){
				sum++;
			}
		}
		printf("%d\n",sum%2);
	}
}
void printrowsandevens(int **dawna){
	printrows(dawna);
	printevens(dawna);
}

REAL anglebetween(REAL x1, REAL y1, REAL xc, REAL yc, REAL x2, REAL y2){
	//angle between two vectors
	REAL v1x = x1 - xc;
	REAL v1y = y1 - yc;
	REAL v2x = x2 - xc;
	REAL v2y = y2 - yc;
	REAL d1 = sqrt(v1x*v1x + v1y*v1y);
	REAL d2 = sqrt(v2x*v2x + v2y*v2y);
	REAL dot = v1x*v2x + v1y*v2y;
	REAL angle = acos(dot/d1/d2);
	return angle*180/3.14159265358;
}

REAL *angles(int *indices, REAL *pointlist, int center_index, int points){
	/*
	returns an array of angles
	angles[n] := the angle between edge n and edge n+1
	*/
	REAL *angles = (REAL*) malloc(points*sizeof(REAL));
	for(int i=0;i<points;i++){
		angles[i] = anglebetween(
						pointlist[2*indices[i]],
						pointlist[2*indices[i]+1],
						pointlist[2*center_index],
						pointlist[2*center_index+1],
						pointlist[2*indices[(i+1)%points]],
						pointlist[2*indices[(i+1)%points]+1]);
	}
	return angles;
}

REAL kawasakisum(REAL *angles, int anglecount, int parity){
	int ind = parity;
	REAL accum = 0; 
	while(ind < anglecount){
		accum += angles[ind];
		ind += 2;
	}
	printf("%f",accum);
	return accum;
}
