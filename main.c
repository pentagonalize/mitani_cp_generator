#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif 
/* not SINGLE */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>
#include <string.h>
#include "triangle.h"
#include "generate_creases.h"


struct triangulateio *in, *out;

int clockwiseof1(const void * a, const void * b){
	int p1 = *(int*)a;
	int p2 = *(int*)b;
 
	REAL x1 = out->pointlist[2*p1] - out->pointlist[10];
	REAL y1 = out->pointlist[2*p1+1] - out->pointlist[11];
	REAL x2 = out->pointlist[2*p2] - out->pointlist[10];
	REAL y2 = out->pointlist[2*p2+1] - out->pointlist[11];
	//this does some checks first, before resorting to calculating crossproduct.
	if (x1 >= 0 && x2 < 0)
        return 1;
    if (x1 < 0 && x2 >= 0)
        return -1;
    if (x1 == 0 && x2 == 0) {
        if (y1 >= 0 || y2 >= 0)
            return 0;
    }
	int crossprod = cross(x1,y1,x2,y2);
	return crossprod;
}

void keyPressed (unsigned char key, int x, int y) {  
	if(key == 'q'){
		printf("dawnaballz\n");
		exit(0);
	}
	if(key == 'n'){

		in = init_in(14);
		out = init_out(in);
		triangulate("pcez", in, out, NULL);
		glutPostRedisplay();
	}
	if(key == 'g'){
		//this should remove edges until satisfies even edges constraint, but it is bad.

		// int **dawna; 
		// int sum = 1;
		// while(sum!=0){
		// 	in = init_in(14);
		// 	out = init_out(in);
		// 	triangulate("pcez", in, out, NULL);
		// 	sum = 0;
		// 	dawna = edgelist_to_matrix(out);
		// 	evenize(dawna,14);
		// 	for(int e=0;e<out->numberofpoints;e++){
		// 		int rowsum = 0;
		// 		for(int f=0;f<out->numberofpoints;f++){
		// 			if(dawna[e][f]==1){
		// 				rowsum++;
		// 			}
		// 		}
		// 		sum+=(rowsum%2);
		// 	}
		// }
		
		// printrowsandevens(dawna);

		// dawna[0][1]=1;
		// dawna[0][3]=1;
		// dawna[1][0]=1;
		// dawna[1][2]=1;
		// dawna[2][1]=1;
		// dawna[2][3]=1;
		// dawna[3][0]=1;
		// dawna[3][2]=1;
		// replace_edgelist_using_matrix(out, dawna);

		// glutPostRedisplay();
	}
}
void display(){
	printf("dawnaball\n");
	//initialize background and colors
	glClear(GL_COLOR_BUFFER_BIT);
	// glColor3ub(rand()%255, rand()%255, rand()%255 );
	glColor3ub(0, 0, 0);

	//draw all the points
	glBegin(GL_POINTS);
	for(int i=0;i<2*out->numberofpoints;i+=2){
		glVertex3f(out->pointlist[i],out->pointlist[i+1],0);
	}
	glEnd();

	//This can be used for dotted lines in the future
	// glPushAttrib(GL_ENABLE_BIT); 
	// glLineStipple(7, 0xAAAA);  
	// glEnable(GL_LINE_STIPPLE);

	//draw all the edges
	for(int i=0;i<2*out->numberofedges;i+=2){
		glBegin(GL_LINES);
		glVertex3f(out->pointlist[2*out->edgelist[i]],out->pointlist[2*out->edgelist[i]+1],0);
		glVertex3f(out->pointlist[2*out->edgelist[i+1]],out->pointlist[2*out->edgelist[i+1]+1],0);
		glEnd();
	}

	//some testing, around the 5th point
	int **edge_matrix; 

	edge_matrix = edgelist_to_matrix(out);
	glPointSize(30);
	glColor3ub(255, 0, 0);
	glBegin(GL_POINTS);
	//draw the 5th point
	glVertex3f(out->pointlist[10],out->pointlist[11],0);
	
	//get all adjacent points to 5th point
	int adj_pts[14];
	int ind = 0;
	for(int i=0;i<14;i++){
		if(edge_matrix[5][i]==1){
			adj_pts[ind]=i;
			ind++;
		}
	}
	glEnd();

	//sort all points around 5th point
	qsort(adj_pts, ind, sizeof(int), clockwiseof1);


	for(int i=0;i<ind;i++){
		//label the points by number in clockwise order
		glBegin(GL_POINTS);
		glColor3ub(255, 0, 0);
		glEnd();
		glVertex3f(out->pointlist[2*adj_pts[i]],out->pointlist[2*adj_pts[i]+1],0);
		glRasterPos3f(out->pointlist[2*adj_pts[i]]+2,out->pointlist[2*adj_pts[i]+1],0);
		char gonad[30];
		sprintf(gonad, "%d",i);
		drawstring(out->pointlist[2*adj_pts[i]],out->pointlist[2*adj_pts[i]+1],0,gonad);
	}
	REAL *adj_angles = angles(adj_pts, out->pointlist, 5, ind);
	for(int i=0;i<ind;i++){
		//label the angles by degree. 
		glRasterPos3f((out->pointlist[2*adj_pts[i]]-out->pointlist[10])/7+out->pointlist[10],
						(out->pointlist[2*adj_pts[(i+1)%ind]] - out->pointlist[11])/7+out->pointlist[11],0);
		char gonad[30];
		sprintf(gonad, "%f",adj_angles[i]);
		drawstring(out->pointlist[2*adj_pts[i]],out->pointlist[2*adj_pts[i]+1],0,gonad);
	}

	//print check of kawaski's theorem
	printf("even %f\n",kawasakisum(adj_angles,ind,0));
	printf("odd %f\n",kawasakisum(adj_angles,ind,1));
	printf("sum %f\n",kawasakisum(adj_angles,ind,0)+kawasakisum(adj_angles,ind,1));

	glEnd();
	glPointSize(15);

	// glPopAttrib();

	glFlush();
}

int main(int argc, char *argv[])
{
	//initialize crease pattern with 14 points 
	in = init_in(14);
	out = init_out(in);
	triangulate("pcez", in, out, NULL);

	//This has no reason to exist
	int duh = 2;
	printf("%d\n",~duh);

	//Initialize display
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1100, 1100);
	glutInitWindowPosition(0, 0);
	//very funny:
    glutCreateWindow("Delawnay Linsdell");
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glPointSize(15);
    glOrtho(-10.f, 110.f, -10.f, 110.f, 0.f, 1.f );
    glutKeyboardFunc(keyPressed);
    glutDisplayFunc(display);
    glutMainLoop();
    //good remember to free MEMORY
    free_t(in);
    free_t(out);
	return 0;
}
