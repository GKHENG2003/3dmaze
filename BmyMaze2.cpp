/*		
 *      Windows Frame Code Was Published By Jeff Molofee 2000. 
 *      The Idea of This Game Was Taken From FPS At GLdomain.com
 *		You Can Reach Me at: gan_kim_heng@yahoo.com.
 */

#include <windows.h>		// Header File For Windows
#include <stdio.h>			// Header File For Standard Input/Output
#include <math.h>		    // Header File For Math functions
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glaux.h>		// Header File For The Glaux Library
#include <stdlib.h>
#include <mmsystem.h> 

#include "resource.h"

#pragma comment( lib, "opengl32.lib")	// Search For OpenGL32.lib While Linking
#pragma comment( lib, "glu32.lib")
#pragma comment( lib, "glaux.lib")
#pragma comment( lib, "winmm.lib")

HDC		hDC=NULL;			// Private GDI Device Context
HGLRC	hRC=NULL;			// Permanent Rendering Context
HWND	hWnd=NULL;			// Holds Our Window Handle

GLdouble WIDTH;
GLdouble HEIGHT;

const CELL = 256;

bool  keys[256];			// Array Used For The Keyboard Routine 
	
bool active=TRUE;			// Window Active Flag Set To TRUE By Default
bool fullscreen=TRUE;		// Fullscreen Flag Set To Fullscreen Mode By Default

BOOL done=FALSE;			// Bool Variable To Exit Loop
bool  gameOver = false;
bool  getOut = false;

GLuint	texture[3];			// Prepare 3 arrays for Bitmap Storage

inline GLdouble ABS(GLdouble A)		// Absolute value function
{
  if (A < 0)
  A = -A; 
  return A;
}

// Hypotenuse Function
inline GLdouble Hypot(GLdouble a, GLdouble b)
{
  return sqrt((a*a)+(b*b));
}

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		// Declaration For WndProc
GLvoid KillGLWindow(GLvoid);

typedef struct												// Create A Structure
{
	GLubyte	*imageData;										// Image Data (Up To 32 Bits)
	GLuint	bpp;											// Image Color Depth In Bits Per Pixel.
	GLuint	width;											// Image Width
	GLuint	height;											// Image Height
	GLuint	texID;											// Texture ID Used To Select A Texture
} TextureImage;												// Structure Name

GLuint		base;											// Base Display List For The Font
TextureImage textures[1];			

bool LoadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type=GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
    
 	if(	texture->width	<=0	||								// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return false;										// Return False
	}

	texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered
	
	if (texture[0].bpp==24)									// Was The TGA 24 Bits
	{
		type=GL_RGB;										// If So Set The 'type' To GL_RGB
	}

   	gluBuild2DMipmaps(GL_TEXTURE_2D, type, texture[0].width, texture[0].height, type, GL_UNSIGNED_BYTE, texture[0].imageData);           

	return true;											// Texture Building Went Ok, Return True
}

GLvoid BuildFont(GLvoid)									// Build Our Font Display List
{
	base=glGenLists(256);									// Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// Select Our Font Texture
	for (int loop1=0; loop1<256; loop1++)					// Loop Through All 256 Lists
	{
		float cx=float(loop1%16)/16.0f;						// X Position Of Current Character
		float cy=float(loop1/16)/16.0f;						// Y Position Of Current Character

		glNewList(base+loop1,GL_COMPILE);					// Start Building A List
			glBegin(GL_QUADS);								// Use A Quad For Each Character
				glTexCoord2f(cx,1.0f-cy-0.0625f);			// Texture Coord (Bottom Left)
				glVertex2d(0,16);							// Vertex Coord (Bottom Left)
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	// Texture Coord (Bottom Right)
				glVertex2i(16,16);							// Vertex Coord (Bottom Right)
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.001f);	// Texture Coord (Top Right)
				glVertex2i(16,0);							// Vertex Coord (Top Right)
				glTexCoord2f(cx,1.0f-cy-0.001f);			// Texture Coord (Top Left)
				glVertex2i(0,0);							// Vertex Coord (Top Left)
			glEnd();										// Done Building Our Quad (Character)
			glTranslated(14,0,0);							// Move To The Right Of The Character
		glEndList();										// Done Building The Display List
	}														// Loop Until All 256 Are Built
}

GLvoid KillFont(GLvoid)										// Delete The Font From Memory
{
	glDeleteLists(base,256);								// Delete All 256 Display Lists
}

GLvoid glPrint(GLint x, GLint y, int set, const char *fmt, ...)	// Where The Printing Happens
{
	char		text[1024];									// Holds Our String
	va_list		ap;											// Pointer To List Of Arguments

	if (fmt == NULL)										// If There's No Text
		return;												// Do Nothing

	va_start(ap, fmt);										// Parses The String For Variables
	    vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
	va_end(ap);												// Results Are Stored In Text

	if (set>1)												// Did User Choose An Invalid Character Set?
	{
		set=1;												// If So, Select Set 1 (Italic)
	}

	glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
	glLoadIdentity();										// Reset The Modelview Matrix
	glTranslated(x,y,-500);									// Position The Text (0,0 - Top Left)
	glRotatef(180,0,0,1);
	glRotatef(180,0,1,0);
	glListBase(base-32+(128*set));							// Choose The Font Set (0 or 1)
	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		// Write The Text To The Screen
	glDisable(GL_TEXTURE_2D);								// Disable Texture Mapping
}

AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

// Create A Structure For The Timer Information
struct
{
  __int64       frequency;								// Timer Frequency
  GLdouble      resolution;								// Timer Resolution
  unsigned long mm_timer_start;     
  
  // Multimedia Timer Start Value
  unsigned long mm_timer_elapsed;						// Multimedia Timer Elapsed Time
  bool			performance_timer;    
  
  // Using The Performance Timer?
  __int64       performance_timer_start;				// Performance Timer Start Value
  __int64       performance_timer_elapsed;				// Performance Timer Elapsed Time
} timer;

// Initialize Our Timer
void TimerInit(void)
{
     memset(&timer, 0, sizeof(timer));   
	 // Clear Our Timer Structure
     // Check To See If A Performance Counter Is Available
     // If One Is Available The Timer Frequency Will Be Updated
     if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))
     {
          // No Performace Counter Available
          timer.performance_timer = FALSE;               // Set Performance Timer To FALSE
          timer.mm_timer_start = timeGetTime();          // Use timeGetTime()
          timer.resolution  = 1.0f/1000.0f;              // Set Our Timer Resolution To .001f
          timer.frequency   = 1000;                      // Set Our Timer Frequency To 1000
          timer.mm_timer_elapsed = timer.mm_timer_start; // Set The Elapsed Time
     }
     else
     {
          // Performance Counter Is Available, Use It Instead Of The Multimedia Timer
          // Get The Current Time And Store It In performance_timer_start
          QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
          timer.performance_timer   = TRUE;    // Set Performance Timer To TRUE
          // Calculate The Timer Resolution Using The Timer Frequency
          timer.resolution    = (GLdouble) (((double)1.0f)/((double)timer.frequency));
          // Set The Elapsed Time To The Current Time
          timer.performance_timer_elapsed = timer.performance_timer_start;
     }
}

// Get Time In Milliseconds
inline GLdouble TimerGetTime()
{
  __int64 time;										  // 'time' Will Hold A 64 Bit Integer
  if (timer.performance_timer)						  // Are We Using The Performance Timer?
  {
    QueryPerformanceCounter((LARGE_INTEGER *) &time); // Current Performance Time
    // Return The Time Elapsed since TimerInit was called
    return ( (GLdouble) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
  }
  else
  {
    // Return The Time Elapsed since TimerInit was called
    return ( (GLdouble) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
  }
}

int LoadGLTextures()									// Load Bitmap And Convert To A Texture
{
  int Status=FALSE;										// Status Indicator
  AUX_RGBImageRec *TextureImage[1];						// Create Storage Space For The Textures
  memset(TextureImage,0,sizeof(void *)*1);				// Set The Pointer To NULL

  if (TextureImage[0]=LoadBMP("texture/wall.bmp"))		// Load Wall
  {
    Status=TRUE;										// Set The Status To TRUE
    glGenTextures(1, &texture[1]);						// Create One Texture

    // Create MipMapped Texture
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);           
  }

  if (TextureImage[0]=LoadBMP("texture/floor.bmp"))		// Load Floor
  {
    Status=TRUE;										// Set The Status To TRUE
    glGenTextures(1, &texture[2]);						// Create One Texture

    // Create MipMapped Texture
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);           
  }

  if (TextureImage[0]=LoadBMP("texture/ceilling.bmp"))	// Load Ceilling
  {
    Status=TRUE;										// Set The Status To TRUE
    glGenTextures(1, &texture[3]);						// Create One Texture

    // Create MipMapped Texture
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);           
  }

  if (TextureImage[0])									// If Texture Exists
	{
    if (TextureImage[0]->data)							// If Texture Image Exists
    {
      free(TextureImage[0]->data);						// Free The Texture Image Memory
    }
    free(TextureImage[0]);								// Free The Image Structure
  }

  return Status;										// Return The Status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	WIDTH=width;
	HEIGHT=height;
	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	glLoadIdentity();									// Reset The Modelview Matrix
	gluPerspective(60.0f,(GLdouble)width/(GLdouble)height,1.0f,1250.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

static GLuint	ROOM[16];								// Storage For The Room Display List

// Build Cube Display Lists
GLvoid BuildLists()
{
    ROOM[0]=glGenLists(16);							// Generate Lists
    glNewList(ROOM[0],GL_COMPILE);			
  
	// Floor
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glBindTexture(GL_TEXTURE_2D, texture[2]);		    
    glBegin(GL_QUADS);
    glTexCoord2d(20,20);  glVertex3d(CELL*4,0,CELL*4);
    glTexCoord2d(20,0);  glVertex3d(CELL*4,0,0);
    glTexCoord2d(0,0);  glVertex3d(0,0,0);
    glTexCoord2d(0,20);  glVertex3d(0,0,CELL*4);
    glEnd();

	// Ceilling
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glBindTexture(GL_TEXTURE_2D, texture[3]);		      
    glBegin(GL_QUADS);
    glTexCoord2d(20,20);  glVertex3d(CELL*4,CELL/4,CELL*4);
    glTexCoord2d(20,0);  glVertex3d(CELL*4,CELL/4,0);
    glTexCoord2d(0,0);  glVertex3d(0,CELL/4,0);
    glTexCoord2d(0,20);  glVertex3d(0,CELL/4,CELL*4);
    glEnd();
/////////////////////////Wall Start///////////////////////////////////	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBindTexture(GL_TEXTURE_2D, texture[1]);     

    //Front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL,CELL/4,0);
    glTexCoord2d(4,0);  glVertex3d(CELL,0,0);
    glTexCoord2d(0,0);  glVertex3d(0,0,0);
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0);
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0,CELL/4,CELL);
    glTexCoord2d(4,0);  glVertex3d(0,0,CELL);
    glTexCoord2d(0,0);  glVertex3d(0,0,0);
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0);
    glEnd();

    glEndList();
//************************************End List 11///////////////////////////////////

	ROOM[1]=ROOM[0]+1;//IDENTITY[IJ=1 2]
	glNewList(ROOM[1],GL_COMPILE);					
   
/////////////////////////Wall Start///////////////////////////////////

    //front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*0.4,CELL/4,0+CELL*1);//change z
    glTexCoord2d(2,0);  glVertex3d(CELL*0.4,0,0+CELL*1);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*1);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*1);//change z
    glEnd();

	//front right wall
	glBegin(GL_QUADS);
	glTexCoord2d(2,1);  glVertex3d(CELL,CELL/4,0+CELL*1);//change z
    glTexCoord2d(2,0);  glVertex3d(CELL,0,0+CELL*1);//change z
    glTexCoord2d(0,0);  glVertex3d(CELL-CELL*0.4,0,0+CELL*1);//change z
    glTexCoord2d(0,1);  glVertex3d(CELL-CELL*0.4,CELL/4,0+CELL*1);//change z
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0,CELL/4,CELL*2);//change z
    glTexCoord2d(4,0);  glVertex3d(0,0,CELL*2);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*1);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*1);//change z
    glEnd();

    glEndList();
//************************************End List 12///////////////////////////////////

	ROOM[2]=ROOM[1]+1;//IDENTITY[IJ=1 3]
	glNewList(ROOM[2],GL_COMPILE);						

/////////////////////////Wall Start///////////////////////////////////

    //front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*0.4,CELL/4,0+CELL*2);//change z
    glTexCoord2d(2,0);  glVertex3d(CELL*0.4,0,0+CELL*2);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*2);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*2);//change z
    glEnd();

	//front right wall
	glBegin(GL_QUADS);
	glTexCoord2d(2,1);  glVertex3d(CELL,CELL/4,0+CELL*2);//change z
    glTexCoord2d(2,0);  glVertex3d(CELL,0,0+CELL*2);//change z
    glTexCoord2d(0,0);  glVertex3d(CELL-CELL*0.4,0,0+CELL*2);//change z
    glTexCoord2d(0,1);  glVertex3d(CELL-CELL*0.4,CELL/4,0+CELL*2);//change z
    glEnd();

	//Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0,CELL/4,CELL*3);//change z
    glTexCoord2d(4,0);  glVertex3d(0,0,CELL*3);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*2);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*2);//change z
    glEnd();

    glEndList();
//************************************End List 13///////////////////////////////////

	ROOM[3]=ROOM[2]+1;//IDENTITY[IJ=1 4]
	glNewList(ROOM[3],GL_COMPILE);						 
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL,CELL/4,0+CELL*3);//change z
    glTexCoord2d(4,0);  glVertex3d(CELL,0,0+CELL*3);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*3);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*3);//change z
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0,CELL/4,CELL*4);//change z
    glTexCoord2d(4,0);  glVertex3d(0,0,CELL*4);//change z
    glTexCoord2d(0,0);  glVertex3d(0,0,0+CELL*3);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,0+CELL*3);//change z
    glEnd();

	//Back Wall  
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL,CELL/4,CELL*4);//change x,z
    glTexCoord2d(4,0);  glVertex3d(CELL,0,CELL*4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0,0,CELL*4);//change z
    glTexCoord2d(0,1);  glVertex3d(0,CELL/4,CELL*4);//change z
    glEnd();

    glEndList();
//************************************End List 14///////////////////////////////////

	ROOM[4]=ROOM[3]+1;//IDENTITY[IJ=2 1]
	glNewList(ROOM[4],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*2,CELL/4,0);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*2,0,0);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0);//change x
    glEnd();

    //Left top abs Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0);//change x
    glEnd();

	//left bottom abs wall
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL);//change x
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL-CELL*0.4);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL-CELL*0.4);//change x,z
    glEnd();

    glEndList();
//************************************End List 21///////////////////////////////////

	ROOM[5]=ROOM[4]+1;//IDENTITY[IJ=2 2]
	glNewList(ROOM[5],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*1+CELL*0.4,CELL/4,0+CELL*1);//change x,z
    glTexCoord2d(2,0);  glVertex3d(CELL*1+CELL*0.4,0,0+CELL*1);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*1);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*1);//change x,z
    glEnd();

	//front right wall
	glBegin(GL_QUADS);
	glTexCoord2d(2,1);  glVertex3d(CELL*2,CELL/4,0+CELL*1);//change x,z
    glTexCoord2d(2,0);  glVertex3d(CELL*2,0,0+CELL*1);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2-CELL*0.4,0,0+CELL*1);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2-CELL*0.4,CELL/4,0+CELL*1);//change x,z
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0+CELL*1,CELL/4,CELL*2);//change x,z
    glTexCoord2d(4,0);  glVertex3d(0+CELL*1,0,CELL*2);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*1);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*1);//change x
    glEnd();

    glEndList();
//************************************End List 22///////////////////////////////////

	ROOM[6]=ROOM[5]+1;//IDENTITY[IJ=2 3]
	glNewList(ROOM[6],GL_COMPILE);							
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*2,CELL/4,0+CELL*2);//change x,z
    glTexCoord2d(4,0);  glVertex3d(CELL*2,0,0+CELL*2);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*2);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*2);//change x,z
    glEnd();

    //Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL*2+CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL*2+CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*2);//change x
    glEnd();

	//left back wall
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL*3);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL*3);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*3-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*3-CELL*0.4);//change x
    glEnd();

    glEndList();
//************************************End List 23///////////////////////////////////

	ROOM[7]=ROOM[6]+1;//IDENTITY[IJ=2 4]
	glNewList(ROOM[7],GL_COMPILE);							
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*1+CELL*0.4,CELL/4,0+CELL*3);//change x,z
    glTexCoord2d(2,0);  glVertex3d(CELL*1+CELL*0.4,0,0+CELL*3);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*3);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*3);//change x,z
    glEnd();

	//front right wall
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*2,CELL/4,0+CELL*3);//change x,z
    glTexCoord2d(2,0);  glVertex3d(CELL*2,0,0+CELL*3);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2-CELL*0.4,0,0+CELL*3);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2-CELL*0.4,CELL/4,0+CELL*3);//change x,z
    glEnd();

    //Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*3);//change x
    glEnd();

	//left back wall
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*1,CELL/4,CELL*4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*1,0,CELL*4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,0+CELL*4-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,0+CELL*4-CELL*0.4);//change x
    glEnd();

	//Back Wall  
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*2,CELL/4,CELL*4);//change x,z
    glTexCoord2d(4,0);  glVertex3d(CELL*2,0,CELL*4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*1,0,CELL*4);//change x,z
    glTexCoord2d(0,1);  glVertex3d(0+CELL*1,CELL/4,CELL*4);//change x,z
    glEnd();

    glEndList();
//************************************End List 24///////////////////////////////////

	ROOM[8]=ROOM[7]+1;//IDENTITY[IJ=3 1]
	glNewList(ROOM[8],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*3,CELL/4,0);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*3,0,0);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,0);//change x
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0+CELL*2,CELL/4,CELL);//change x,z
    glTexCoord2d(4,0);  glVertex3d(0+CELL*2,0,CELL);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,0);//change x
    glEnd();

    glEndList();
//************************************End List 31///////////////////////////////////

	ROOM[9]=ROOM[8]+1;//IDENTITY[IJ=3 2]
	glNewList(ROOM[9],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*2+CELL*0.4,CELL/4,0+CELL*1);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*2+CELL*0.4,0,0+CELL*1);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,0+CELL*1);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,0+CELL*1);//change x
    glEnd();

	//front right
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*3,CELL/4,0+CELL*1);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*3,0,0+CELL*1);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3-CELL*0.4,0,0+CELL*1);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3-CELL*0.4,CELL/4,0+CELL*1);//change x
    glEnd();
    
	//Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*2,CELL/4,CELL+CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*2,0,CELL+CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL);//change x
    glEnd();

	//left back
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*2,CELL/4,CELL*2);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*2,0,CELL*2);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL*2-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL*2-CELL*0.4);//change x
    glEnd();

    glEndList();
//************************************End List 32///////////////////////////////////

	ROOM[10]=ROOM[9]+1;//IDENTITY[IJ=3 3]
	glNewList(ROOM[10],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*2+CELL*0.4,CELL/4,0+CELL*2);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*2+CELL*0.4,0,0+CELL*2);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,0+CELL*2);//change x
    glEnd();

	//front right
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*3,CELL/4,0+CELL*2);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*3,0,0+CELL*2);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3-CELL*0.4,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3-CELL*0.4,CELL/4,0+CELL*2);//change x
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0+CELL*2,CELL/4,CELL*3);//change x,z
    glTexCoord2d(4,0);  glVertex3d(0+CELL*2,0,CELL*3);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL*2);//change x
    glEnd();

    glEndList();
//************************************End List 33///////////////////////////////////

	ROOM[11]=ROOM[10]+1;//IDENTITY[IJ=3 4]
	glNewList(ROOM[11],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*3,CELL/4,0+CELL*3);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*3,0,0+CELL*3);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,0+CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,0+CELL*3);//change x
    glEnd();

    //Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*2,CELL/4,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*2,0,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL*3);//change x
    glEnd();

	//left back
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*2,CELL/4,CELL*4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*2,0,CELL*4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL*4-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL*4-CELL*0.4);//change x
    glEnd();
	
	//Back Wall  
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*3,CELL/4,CELL*4);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*3,0,CELL*4);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*2,0,CELL*4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*2,CELL/4,CELL*4);//change x
    glEnd();

    glEndList();
//************************************End List 34///////////////////////////////////

	ROOM[12]=ROOM[11]+1;//IDENTITY[IJ=4 1]
	glNewList(ROOM[12],GL_COMPILE);							
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,0);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,0);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,0);//change x
    glEnd();

    //Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*3,CELL/4,CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*3,0,CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,0);//change x
    glEnd();

	//left back 
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*3,CELL/4,CELL);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*3,0,CELL);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL-CELL*0.4);//change x
    glEnd();

	//Right Wall    	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,CELL);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,CELL);//change x
    glTexCoord2d(0,0);  glVertex3d(CELL*4,0,0);//change x
    glTexCoord2d(0,1);  glVertex3d(CELL*4,CELL/4,0);//change x
    glEnd();

    glEndList();
//************************************End List 41///////////////////////////////////

	ROOM[13]=ROOM[12]+1;//IDENTITY[IJ=4 2]
	glNewList(ROOM[13],GL_COMPILE);							
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front wall
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,0+CELL*1);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,0+CELL*1);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,0+CELL*1);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,0+CELL*1);//change x
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0+CELL*3,CELL/4,CELL*2);//change x,z
    glTexCoord2d(4,0);  glVertex3d(0+CELL*3,0,CELL*2);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL);//change x
    glEnd();

	//Right front Wall    	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*4,CELL/4,CELL+CELL*0.4);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*4,0,CELL+CELL*0.4);//change x
    glTexCoord2d(0,0);  glVertex3d(CELL*4,0,0+CELL*1);//change x
    glTexCoord2d(0,1);  glVertex3d(CELL*4,CELL/4,0+CELL*1);//change x
    glEnd();

	//right back
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*4,CELL/4,CELL*2);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*4,0,CELL*2);//change x
    glTexCoord2d(0,0);  glVertex3d(CELL*4,0,0+CELL*2-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(CELL*4,CELL/4,0+CELL*2-CELL*0.4);//change x
    glEnd();

    glEndList();
//************************************End List 42///////////////////////////////////

	ROOM[14]=ROOM[13]+1;//IDENTITY[IJ=4 3]
	glNewList(ROOM[14],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*3+CELL*0.4,CELL/4,0+CELL*2);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*3+CELL*0.4,0,0+CELL*2);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,0+CELL*2);//change x
    glEnd();

	//front right
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*4,CELL/4,0+CELL*2);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*4,0,0+CELL*2);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*4-CELL*0.4,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*4-CELL*0.4,CELL/4,0+CELL*2);//change x
    glEnd();

    //Left Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(0+CELL*3,CELL/4,CELL*3);//change x,z
    glTexCoord2d(4,0);  glVertex3d(0+CELL*3,0,CELL*3);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL*2);//change x
    glEnd();

	//Right Wall    	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,CELL*3);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,CELL*3);//change x
    glTexCoord2d(0,0);  glVertex3d(CELL*4,0,0+CELL*2);//change x
    glTexCoord2d(0,1);  glVertex3d(CELL*4,CELL/4,0+CELL*2);//change x
    glEnd();

    glEndList();
//************************************End List 43///////////////////////////////////

	ROOM[15]=ROOM[14]+1;//IDENTITY[IJ=4 4]
	glNewList(ROOM[15],GL_COMPILE);						
   
/////////////////////////Wall Start///////////////////////////////////	
	
	//front left wall
    glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*3+CELL*0.4,CELL/4,0+CELL*3);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*3+CELL*0.4,0,0+CELL*3);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,0+CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,0+CELL*3);//change x
    glEnd();

	//front right
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(CELL*4,CELL/4,0+CELL*3);//change x
    glTexCoord2d(2,0);  glVertex3d(CELL*4,0,0+CELL*3);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*4-CELL*0.4,0,0+CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*4-CELL*0.4,CELL/4,0+CELL*3);//change x
    glEnd();

    //Left front Wall   	
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*3,CELL/4,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*3,0,CELL*3+CELL*0.4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL*3);//change x
    glEnd();

	//left back
	glBegin(GL_QUADS);
    glTexCoord2d(2,1);  glVertex3d(0+CELL*3,CELL/4,CELL*4);//change x,z
    glTexCoord2d(2,0);  glVertex3d(0+CELL*3,0,CELL*4);//change x,z
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL*4-CELL*0.4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL*4-CELL*0.4);//change x
    glEnd();

	//Right Wall    	
	glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,CELL*4);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,CELL*4);//change x
    glTexCoord2d(0,0);  glVertex3d(CELL*4,0,0+CELL*3);//change x
    glTexCoord2d(0,1);  glVertex3d(CELL*4,CELL/4,0+CELL*3);//change x
    glEnd();

	//Back Wall  
    glBegin(GL_QUADS);
    glTexCoord2d(4,1);  glVertex3d(CELL*4,CELL/4,CELL*4);//change x
    glTexCoord2d(4,0);  glVertex3d(CELL*4,0,CELL*4);//change x
    glTexCoord2d(0,0);  glVertex3d(0+CELL*3,0,CELL*4);//change x
    glTexCoord2d(0,1);  glVertex3d(0+CELL*3,CELL/4,CELL*4);//change x
    glEnd();

    glEndList();
//************************************End List 44///////////////////////////////////
}

GLdouble Time1;
GLdouble Time2;
GLdouble DiffTime;
GLdouble StartTime;

int InitGL(GLvoid)											// All Setup For OpenGL Goes Here
{ 
	if (!LoadTGA(&textures[0],"texture/Font.TGA"))			// Load The Font Texture
	{
		return false;										// If Loading Failed, Return False
	}
	
	if (!LoadGLTextures())									// Jump To Texture Loading Routine
      return false;  

	BuildLists();

	glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
	glClearColor(0, 0, 0, 1.0f);							// Black Background
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Really Nice Perspective Calculations
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	TimerInit();											//initialize timer
	BuildFont();   
	StartTime = TimerGetTime()/1000; 

	return TRUE;											// Initialization Went OK
}

GLdouble xtrans2 = CELL*2 + CELL/3;
GLdouble ztrans2 = CELL*2 + CELL/1.5f;

GLdouble xtrans = 0;
GLdouble ztrans = 0;

const GLdouble piover180 = 0.0174532925f;
GLdouble XP=0;
GLdouble ZP=0;
GLdouble yrot=0;										
GLdouble sceneroty;
GLdouble heading;
GLdouble zprot;

int frames = 0;
GLdouble FPS = 0;

inline bool DetectCollision(GLdouble &cx, GLdouble &cz, GLdouble &cxi, GLdouble &czi, GLdouble padding, GLdouble bounce)
{
	bool Status = false;

	if(ztrans2>0 && ztrans2<CELL && //********************Range for 11
		xtrans2>0 && xtrans2<CELL)
	{
		if (cx > CELL-padding && 
			(cz < CELL*0.4+padding/2.0 || 
				cz > CELL-CELL*0.4-padding/2.0))//[1 1]right door
		{	
			cx = CELL-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < 0+padding)
		{
			cx = 0+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL-padding && 
			(cx < CELL*0.4+padding/2.0 || 
				cx > CELL-CELL*0.4-padding/2.0))//[1 1]back door	
		{
			cz = CELL-padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < 0+padding)		
		{
			cz = 0+padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*1 && ztrans2<CELL*2 && //********************Range for 12
			xtrans2>0 && xtrans2<CELL)//x const
	{
		if (cx > CELL-padding)
		{	
			cx = CELL-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < 0+padding)
		{
			cx = 0+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*2-padding && 
			(cx < CELL*0.4+padding/2.0 || 
				cx > CELL*1-CELL*0.4-padding/2.0))//[1 2]back door	
		{
			cz = CELL*2-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*1+padding && 
				(cx < CELL*0.4+padding/2.0 || 
					cx > CELL*1-CELL*0.4-padding/2.0))//[1 2]front door		
		{
			cz = CELL*1+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*2 && ztrans2<CELL*3 && //********************Range for 13 
			xtrans2>0 && xtrans2<CELL)//x const
	{
		if (cx > CELL-padding && 
			(cz < CELL*2+CELL*0.4+padding/2.0 || 
				cz > CELL*3-CELL*0.4-padding/2.0))//[1 3]right door
		{	
			cx = CELL-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < 0+padding)
		{
			cx = 0+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*3-padding)	
		{
			cz = CELL*3-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*2+padding && 
				(cx < CELL*0.4+padding/2.0 || 
					cx > CELL*1-CELL*0.4-padding/2.0))//[1 3]front door		
		{
			cz = CELL*2+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*3 && ztrans2<CELL*4 && //********************Range for 14
			xtrans2>0 && xtrans2<CELL)//x const
	{
		if (cx > CELL-padding && 
			(cz < CELL*3+CELL*0.4+padding/2.0 || 
				cz > CELL*4-CELL*0.4-padding/2.0))//[1 4]right door
		{	
			cx = CELL-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < 0+padding)
		{	
			cx = 0+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*4-padding)	
		{
			cz = CELL*4-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*3+padding)	
		{
			cz = CELL*3+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>0 && ztrans2<CELL && //********************Range for 21
			xtrans2>CELL*1 && xtrans2<CELL*2)
	{
		if (cx > CELL*2-padding)
		{	
			cx = CELL*2-padding;//change with statement
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*1+padding && 
				(cz < CELL*0+CELL*0.4+padding/2.0 || 
					cz > CELL*1-CELL*0.4-padding/2.0))//[2 1]left door
		{
			cx = CELL*1+padding;//change with statement
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL-padding && 
			(cx < CELL*1+CELL*0.4+padding/2.0 || 
				cx > CELL*2-CELL*0.4-padding/2.0))//[2 1]back door	
		{
			cz = CELL-padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < 0+padding)		
		{
			cz = 0+padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*1 && ztrans2<CELL*2 && //********************Range for 22
			xtrans2>CELL*1 && xtrans2<CELL*2)//z const
	{
		if (cx > CELL*2-padding && 
			(cz < CELL*1+CELL*0.4+padding/2.0 || 
				cz > CELL*2-CELL*0.4-padding/2.0))//[2 2]right door
		{	
			cx = CELL*2-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*1+padding)
		{
			cx = CELL*1+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*2-padding)	
		{
			cz = CELL*2-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*1+padding && 
				(cx < CELL*1+CELL*0.4+padding/2.0 || 
					cx > CELL*2-CELL*0.4-padding/2.0))//[2 2]front door		
		{
			cz = CELL*1+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*2 && ztrans2<CELL*3 && //********************Range for 23
			xtrans2>CELL*1 && xtrans2<CELL*2)//z const
	{
		if (cx > CELL*2-padding)
		{	
			cx = CELL*2-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*1+padding  && 
				(cz < CELL*2+CELL*0.4+padding/2.0 || 
					cz > CELL*3-CELL*0.4-padding/2.0))//[2 3]left door
		{
			cx = CELL*1+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*3-padding && 
			(cx < CELL*1+CELL*0.4+padding/2.0 || 
				cx > CELL*2-CELL*0.4-padding/2.0))//[2 3]back door	
		{
			cz = CELL*3-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*2+padding) 		
		{
			cz = CELL*2+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*3 && ztrans2<CELL*4 && //********************Range for 24
			xtrans2>CELL*1 && xtrans2<CELL*2)//z const
	{
		if (cx > CELL*2-padding && 
			(cz < CELL*3+CELL*0.4+padding/2.0 || 
				cz > CELL*4-CELL*0.4-padding/2.0))//[2 4]right door
		{	
			cx = CELL*2-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*1+padding && 
				(cz < CELL*3+CELL*0.4+padding/2.0 || 
					cz > CELL*4-CELL*0.4-padding/2.0))//[2 4]left door
		{
			cx = CELL*1+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*4-padding)	
		{
			cz = CELL*4-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*3+padding && 
				(cx < CELL*1+CELL*0.4+padding/2.0 || 
					cx > CELL*2-CELL*0.4-padding/2.0))//[2 4]front door		
		{
			cz = CELL*3+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>0 && ztrans2<CELL && //********************Range for 31
			xtrans2>CELL*2 && xtrans2<CELL*3)
	{
		if (cx > CELL*3-padding && 
			(cz < CELL*0+CELL*0.4+padding/2.0 || 
				cz > CELL*1-CELL*0.4-padding/2.0))//[3 1]right door
		{	
			cx = CELL*3-padding;//change with statement
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*2+padding) 
		{
			cx = CELL*2+padding;//change with statement
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL-padding && 
			(cx < CELL*2+CELL*0.4+padding/2.0 || 
				cx > CELL*3-CELL*0.4-padding/2.0))//[3 1]back door	
		{
			cz = CELL-padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < 0+padding)		
		{
			cz = 0+padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*1 && ztrans2<CELL*2 && //********************Range for 32
			xtrans2>CELL*2 && xtrans2<CELL*3)//z const
	{
		if (cx > CELL*3-padding) 
		{	
			cx = CELL*3-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*2+padding && 
				(cz < CELL*1+CELL*0.4+padding/2.0 || 
					cz > CELL*2-CELL*0.4-padding/2.0))//[3 2]left door
		{
			cx = CELL*2+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*2-padding && 
			(cx < CELL*2+CELL*0.4+padding/2.0 || 
				cx > CELL*3-CELL*0.4-padding/2.0))//[3 2]back door	
		{
			cz = CELL*2-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*1+padding && 
				(cx < CELL*2+CELL*0.4+padding/2.0 || 
					cx > CELL*3-CELL*0.4-padding/2.0))//[3 2]front door		
		{
			cz = CELL*1+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*2 && ztrans2<CELL*3 && //********************Range for 33
			xtrans2>CELL*2 && xtrans2<CELL*3)//z const
	{
		if (cx > CELL*3-padding)
		{	
			cx = CELL*3-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*2+padding)
		{
			cx = CELL*2+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*3-padding) 	
		{
			cz = CELL*3-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*2+padding && 
				(cx < CELL*2+CELL*0.4+padding/2.0 || 
					cx > CELL*3-CELL*0.4-padding/2.0))//[3 3]front door 		
		{
			cz = CELL*2+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*3 && ztrans2<CELL*4 && //********************Range for 34
			xtrans2>CELL*2 && xtrans2<CELL*3)//z const
	{
		if (cx > CELL*3-padding && 
			(cz < CELL*3+CELL*0.4+padding/2.0 || 
				cz > CELL*4-CELL*0.4-padding/2.0))//[3 4]right door
		{	
			cx = CELL*3-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*2+padding && 
				(cz < CELL*3+CELL*0.4+padding/2.0 || 
					cz > CELL*4-CELL*0.4-padding/2.0))//[3 4]left door
		{
			cx = CELL*2+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*4-padding)	
		{
			cz = CELL*4-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*3+padding)
		{
			cz = CELL*3+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>0 && ztrans2<CELL && //********************Range for 41
			xtrans2>CELL*3 && xtrans2<CELL*4)
	{
		if (cx > CELL*4-padding)
		{	
			cx = CELL*4-padding;//change with statement
			cxi = -cxi;
		    cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*3+padding && 
				(cz < CELL*0+CELL*0.4+padding/2.0 || 
					cz > CELL*1-CELL*0.4-padding/2.0))//[4 1]left door
		{
			cx = CELL*3+padding;//change with statement
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL-padding)
		{
			cz = CELL-padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < 0+padding)		
		{
			cz = 0+padding;
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*1 && ztrans2<CELL*2 && //********************Range for 42
			xtrans2>CELL*3 && xtrans2<CELL*4)//z const
	{
		if (cx > CELL*4-padding && 
			(cz < CELL*1+CELL*0.4+padding/2.0 || 
				cz > CELL*2-CELL*0.4-padding/2.0))//[4 2]right door 
		{	
			cx = CELL*4-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*3+padding) 
		{
			cx = CELL*3+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
			}

		if (cz > CELL*2-padding && 
			(cx < CELL*3+CELL*0.4+padding/2.0 || 
				cx > CELL*4-CELL*0.4-padding/2.0))//[4 2]back door	
		{
			cz = CELL*2-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*1+padding)		
		{
			cz = CELL*1+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*2 && ztrans2<CELL*3 && //********************Range for 43
			xtrans2>CELL*3 && xtrans2<CELL*4)//z const
	{
		if (cx > CELL*4-padding)
		{	
			cx = CELL*4-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*3+padding)
		{
			cx = CELL*3+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*3-padding && 
			(cx < CELL*3+CELL*0.4+padding/2.0 || 
				cx > CELL*4-CELL*0.4-padding/2.0))//[4 3]back door 	 	
		{
			cz = CELL*3-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*2+padding && 
				(cx < CELL*3+CELL*0.4+padding/2.0 || 
					cx > CELL*4-CELL*0.4-padding/2.0))//[4 3]front door 		
		{
			cz = CELL*2+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	else if(ztrans2>CELL*3 && ztrans2<CELL*4 && //********************Range for 44
			xtrans2>CELL*3 && xtrans2<CELL*4)//z const
	{
		if (cx > CELL*4-padding)
		{	
			cx = CELL*4-padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}
		else if (cx < CELL*3+padding && 
				(cz < CELL*3+CELL*0.4+padding/2.0 || 
					cz > CELL*4-CELL*0.4-padding/2.0))//[4 4]left door
		{
			cx = CELL*3+padding;
			cxi = -cxi;
			cxi *= bounce; 
			Status = true;
		}

		if (cz > CELL*4-padding)	
		{
			cz = CELL*4-padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
		else if (cz < CELL*3+padding && 
				(cx < CELL*3+CELL*0.4+padding/2.0 || 
					cx > CELL*4-CELL*0.4-padding/2.0))//[4 3]front door
		{
			cz = CELL*3+padding;//change with statement
			czi = -czi;
			czi *= bounce; 
			Status = true;
		}
	}
	return Status;
}

inline int DrawGLScene(GLvoid)								// Here's Where We Do All The Drawing
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	if(getOut == false && gameOver == false)
	{
		XP *= 0.8f; 
		ZP *= 0.8f; 
		xtrans2 += XP/10;
		ztrans2 += ZP/10;

		xtrans = -xtrans2;
		ztrans = -ztrans2;
		
		DetectCollision(xtrans2, ztrans2, XP, ZP, 5.0f, 0);

		zprot*=.9f; 
		heading += zprot;
		yrot = heading;	
		sceneroty = 360.0f - yrot;

		glLoadIdentity();
			glRotated(sceneroty,0,1.f,0);
			glTranslated(xtrans,-10,ztrans);

		for(int i=0;i<16;i++)
		{
			glCallList(ROOM[i]);
		}

		//  ESTABLISH A FPS FRAMERATE COUNTER
		glDisable(GL_DEPTH_TEST);
		frames++;    
		if (frames%10 == 0) 
		{
			Time2 = TimerGetTime()/1000;
			DiffTime = ABS(Time2-Time1);      
			Time1 = TimerGetTime()/1000;     
			if (DiffTime != 0)
				FPS = 10/(DiffTime);
		}             
    
		glBindTexture(GL_TEXTURE_2D, textures[0].texID);	// Select Our Font Texture 
		glPrint(-350,250,1,"FPS: %4.0f", FPS);
		glPrint(-350,230,1,"Distance: %4.0f", Hypot( (xtrans2-CELL*4),(ztrans2-CELL*3/2.0) )/10.0 );					
		glPrint(-350,210,1,"Time Left: %4.0f s", 500.0f - (Time1-StartTime) );
//******************************* End ***************************************************
		// If player get out
		if( Hypot( (xtrans2-CELL*4),(ztrans2-CELL*3/2.0) )/10.0f < 5.0f )
			getOut = true; 
		// If time's up
		if( 500.0f - (Time1-StartTime) < 0.0f )
			gameOver = true;
//******************************* End ***************************************************

		glEnable(GL_DEPTH_TEST);           
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
	}else if(gameOver == true) // If game over
	{
		glBindTexture(GL_TEXTURE_2D, textures[0].texID);	// Select Our Font Texture
			glColor4d(1,0,0,1);	
			glPrint(-50,90,1,"Time's Up!"); 
			glColor4d(1,1,1,1);
			glPrint(-50,50,1,"Press ESC to exit...");
		glEnable(GL_DEPTH_TEST);           
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
	}else // If palyer get out 
	{
		glBindTexture(GL_TEXTURE_2D, textures[0].texID);	// Select Our Font Texture	 
			glColor4d(0,0,1,1);	
			glPrint(-50,90,1,"You win!"); 
			glColor4d(1,1,1,1);
			glPrint(-50,50,1,"Press ESC to exit...");
		glEnable(GL_DEPTH_TEST);           
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
	}

	return TRUE;											// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	KillFont();
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	HINSTANCE	hInstance;				// Holds The Instance Of The Application
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style


	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );	
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","Simple 3D Maze",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;						// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	// Windows Style
	}

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,			// Extended Style For The Window
								"OpenGL",			// Class Name
								title,				// Window Title
								dwStyle,			// Window Style
								0, 0,				// Window Position
								width, height,		// Selected Width And Height
								NULL,				// No Parent Window
								NULL,				// No Menu
								hInstance,			// Instance
								NULL)))				// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{	 
	if (uMsg == WM_ACTIVATE)						// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))						// Check Minimization State
		{
			active=TRUE;							// Program Is Active
		}
		else
		{
			active=FALSE;							// Program Is No Longer Active
		}
	}
    else if (uMsg == WM_SYSCOMMAND)					// Intercept System Commands
	{
		switch (wParam)								// Check System Calls
		{
			case SC_SCREENSAVE:						// Screensaver Trying To Start?
			case SC_MONITORPOWER:					// Monitor Trying To Enter Powersave?
				
			return 0;								// Prevent From Happening
		}
	}
	else if (uMsg == WM_CLOSE)						// Did We Receive A Close Message?
	{
		PostQuitMessage(0);							// Send A Quit Message
	}
	else if (uMsg == WM_KEYDOWN)					// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;						// If So, Mark It As TRUE
	}
	else if (uMsg == WM_KEYUP)						// Has A Key Been Released?
	{
		keys[wParam] = FALSE;						// If So, Mark It As FALSE
	}
	else if (uMsg == WM_SIZE)						// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
	}        		

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	
	MSG		msg;									// Windows Message Structure

	// Ask The User Which Screen Mode They Prefer  
	fullscreen=true;

	if (!CreateGLWindow("Simple 3D Maze ",800,600,32,fullscreen))
	{
		return 0;
	}
	
	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
				
				if (keys[VK_UP])  // Move forwards
				{
					XP -= (GLdouble)sin(heading*piover180) * 10.0f;	
					ZP -= (GLdouble)cos(heading*piover180) * 10.0f;
				}
				else if (keys['W'])  // Move forwards
				{
					XP -= (GLdouble)sin(heading*piover180) * 10.0f;	
					ZP -= (GLdouble)cos(heading*piover180) * 10.0f;
				}
				if (keys[VK_DOWN]) // Move backwards
				{
					XP += (GLdouble)sin(heading*piover180) * 10.0f;	
					ZP += (GLdouble)cos(heading*piover180) * 10.0f;
				}	               
				else if (keys['S']) // Move backwards
				{
					XP += (GLdouble)sin(heading*piover180) * 10.0f;	
					ZP += (GLdouble)cos(heading*piover180) * 10.0f;
				}					
				if (keys['A'])  // strafe left
				{
					XP += (GLdouble)sin((heading-90)*piover180) * 10.0f;	
					ZP += (GLdouble)cos((heading-90)*piover180) * 10.0f;
				}
				if (keys['D']) // strafe right
				{
					XP += (GLdouble)sin((heading+90)*piover180) * 10.0f;	
					ZP += (GLdouble)cos((heading+90)*piover180) * 10.0f;
				}	               		
				if (keys[VK_LEFT]) // Turn left
				{
					zprot += 0.5f;
				}
				else if (keys[VK_RIGHT]) // Turn right
				{
					zprot -= 0.5f;
				} 
			}
		}
	}
	// Shutdown
	KillGLWindow();									// Kill The Window

	return (msg.wParam);							// Exit The Program
}