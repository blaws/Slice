// blaws
// Slice.c

#include <iostream>

#include <cstdlib>
#include <cmath>
#include <deque>
#include <GLUT/glut.h>
using namespace std;

int wsize=500;
int mouseDown,mouseCoords[2],lastCoords[2],newVert[2],newOrder[2];
int screenArray[500][500],sliceReady;
float colorScreenArray[500][500][3];
deque<std::deque<int> > vertices;
deque<int> placeholder;

void display(void){
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  /*for(unsigned int i=0;i<vertices.size();i++){
    glColor4f(i==0||i==3||i==5||i==6,i==1||i==3||i==4||i==6,i==2||i==4||i==5||i==6,0.5);
    glBegin(GL_POLYGON);
    for(unsigned int j=0;j<vertices[i].size();j+=2){
      glVertex2i(vertices[i][j],vertices[i][j+1]);
    }
    glEnd();
    }*/

  for(int i=0;i<wsize;i++){
    for(int j=0;j<wsize;j++){
      int k = (screenArray[j][i]-1) % 7;
      float div = screenArray[j][i]/7 + 1;
      colorScreenArray[i][j][0] = (k==0||k==3||k==5||k==6) / div;
      colorScreenArray[i][j][1] = (k==1||k==3||k==4||k==6) / fmax(div-1,1);
      colorScreenArray[i][j][2] = (k==2||k==4||k==5||k==6) / fmax(div-2,1);
    }
  }
  glRasterPos2i(0,0);
  glDrawPixels(wsize,wsize,GL_RGB,GL_FLOAT,colorScreenArray);

  if(mouseDown){
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_LINES);
    glVertex2iv(mouseCoords);
    glVertex2iv(lastCoords);
    glEnd();
  }

  glutSwapBuffers();
}

void reshape(int w,int h){
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,w,0,h);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void breakPoly(int x,int y,int currentPoly,double angle){
  int offset1,offset2;  // to prevent divide by 0
  for(unsigned int i=0;i<vertices.size();i++){
    for(unsigned int j=0;j<vertices[i].size();j+=2){
      if(vertices[i][j]-newVert[0]==0) offset1=1;
      else offset1=0;
      if(newVert[0]-vertices[i][(j+2)%vertices[i].size()]==0) offset2=1;
      else offset2=0;
      if(((vertices[i][j]-newVert[0]==0)&&(newVert[0]-vertices[i][(j+2)%vertices[i].size()]==0)) || ((vertices[i][j+1]-newVert[1])/(offset1+vertices[i][j]-newVert[0]) == (newVert[1]-vertices[i][(j+3)%vertices[i].size()])/(offset2+newVert[0]-vertices[i][(j+2)%vertices[i].size()]))){
	newOrder[0] = j+2;
      }
      else{
	if(vertices[i][j]-x==0) offset1=1;
	else offset1=0;
	if(x-vertices[i][(j+2)%vertices[i].size()]==0) offset2=1;
	else offset2=0;
	if(((vertices[i][j]-x==0)&&(x-vertices[i][(j+2)%vertices[i].size()]==0)) || ((vertices[i][j+1]-y)/(offset1+vertices[i][j]-x) == (y-vertices[i][(j+3)%vertices[i].size()])/(offset2+x-vertices[i][(j+2)%vertices[i].size()]))){
	  newOrder[1] = j+2;
	}
      }
    }
    for(int j=0;j<2;j++){
      if(newOrder[j]==8) newOrder[j]=4;
      else if(newOrder[j]==4) newOrder[j]=8;
    }

    // new shape
    vertices.push_back(placeholder);
    for(int j=0;j<min(newOrder[0],newOrder[1]);j++){
      vertices[vertices.size()-1].push_back(vertices[i][j]);
    }
    vertices[vertices.size()-1].push_back(newOrder[0]<newOrder[1]?newVert[0]:x);
    vertices[vertices.size()-1].push_back(newOrder[0]<newOrder[1]?newVert[1]:y);
    vertices[vertices.size()-1].push_back(newOrder[0]<newOrder[1]?x:newVert[0]);
    vertices[vertices.size()-1].push_back(newOrder[0]<newOrder[1]?y:newVert[1]);
    for(unsigned int j=max(newOrder[0],newOrder[1]);j<vertices[i].size();j++){
      vertices[vertices.size()-1].push_back(vertices[i][j]);
    }

    // original shape
    for(unsigned int k=0;k<vertices[vertices.size()-1].size();k+=2){
      for(unsigned int j=0;j<vertices[i].size();j+=2){
	if(vertices[i][j]==vertices[vertices.size()-1][k] && vertices[i][j+1]==vertices[vertices.size()-1][k+1]){
	  for(int m=0;m<2;m++) vertices[i].erase(vertices[i].begin()+j);
	  break;
	}
      }
    }
    vertices[i].push_front(newOrder[0]<newOrder[1]?newVert[1]:y);
    vertices[i].push_front(newOrder[0]<newOrder[1]?newVert[0]:x);
    vertices[i].push_back(newOrder[0]<newOrder[1]?x:newVert[0]);
    vertices[i].push_back(newOrder[0]<newOrder[1]?y:newVert[1]);

    // update screenArray
    int yInter = y-angle*x;
    for(int j=0;j<wsize;j++){
      for(int k=0;k<wsize;k++){
	if(screenArray[j][k] == currentPoly){
	  if(k > angle*j+yInter) screenArray[j][k]=vertices.size();
	}
      }
    }
    return;
  }
}

void traceSlice(int x,int y){
  double i = min(mouseCoords[0],x);
  double j = (mouseCoords[0]<x ? mouseCoords[1] : y);
  double angle = ((double)y-mouseCoords[1]) / (x-mouseCoords[0]);
  int currentNum = screenArray[(int)i][(int)j];
  double iInc,jInc;

  if(angle<=1.0 && angle>=-1.0) iInc=1.0,jInc=angle;
  else iInc=fabs(1.0/angle),jInc=angle>0.0?1.0:-1.0;

  for(;i<max(x,mouseCoords[0]);i+=iInc,j+=jInc){
    if(i>=0 && i<wsize && j>=0 && j<wsize){
      if(screenArray[(int)i][(int)j] != currentNum){
	if(newVert[0]!=-1 && newVert[1]!=-1){
	  breakPoly((int)i,(int)j,currentNum,angle);
	}
	newVert[0] = (int)i;
	newVert[1] = (int)j;
	currentNum = screenArray[(int)i][(int)j];
      }
    }
  }
  newVert[0] = newVert[1] = -1;
}

void mouse(int button,int state,int x,int y){
  y = wsize-y;
  switch(button){
  case GLUT_LEFT_BUTTON:
    if(state==GLUT_DOWN){
      mouseCoords[0] = x;
      mouseCoords[1] = y;
      mouseDown = 1;
    }
    else{
      mouseDown = 0;
      if(x==mouseCoords[0]) mouseCoords[0]--;
      if(y==mouseCoords[1]) mouseCoords[1]--;
      if(sliceReady) traceSlice(x,y);
      //cout<<"poly "<<screenArray[x][y]<<endl;
      glutPostRedisplay();
    }
  default:
    break;
  }
}

void mouseMove(int x,int y){
  y = wsize-y;
  lastCoords[0] = x;
  lastCoords[1] = y;
  glutPostRedisplay();
}

void init(){
  sliceReady = 1;
  newVert[0] = newVert[1] = -1;
  vertices.clear();
  vertices.push_back(placeholder);
  for(int i=0;i<4;i++){
    vertices[0].push_back(i==1||i==2 ? 3*wsize/4 : wsize/4);
    vertices[0].push_back(i/2 ? 3*wsize/4 : wsize/4);
  }
  for(int i=0;i<wsize;i++){
    for(int j=0;j<wsize;j++){
      if(i>wsize/4 && i<3*wsize/4 && j>wsize/4 && j<3*wsize/4){
	screenArray[i][j] = 1;
      }
      else screenArray[i][j] = 0;
    }
  }
}

void keyboard(unsigned char key,int x,int y){
  switch(key){
  case 'q':
  case 'Q':
  case 27:
    exit(0);
    break;
  case 'r':
  case 'R':
    init();
    break;
  default:
    break;
  }
  glutPostRedisplay();
}

int main(int argc,char** argv){
  glutInit(&argc,argv);
  glutInitWindowSize(wsize,wsize);
  glutInitWindowPosition((1280-wsize)/2-150,(800-wsize)/2);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutCreateWindow("Slice");

  init();

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMove);

  glutMainLoop();
  return 0;
}
