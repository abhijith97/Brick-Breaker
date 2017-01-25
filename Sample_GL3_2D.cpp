#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ao/ao.h>
// #include <FTGL/ftgl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


    #define trace1(x)                cerr << #x << ": " << x << endl;
    #define trace2(x, y)             cerr << #x << ": " << x << " | " << #y << ": " << y << endl;
    #define trace3(x, y, z)          cerr << #x << ": " << x << " | " << #y << ": " << y << " | " << #z << ": " << z << endl;
    #define trace4(a, b, c, d)       cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << endl;
    #define trace5(a, b, c, d, e)    cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << " | " << #e << ": " << e << endl;
    #define trace6(a, b, c, d, e, f) cerr << #a << ": " << a << " | " << #b << ": " << b << " | " << #c << ": " << c << " | " << #d << ": " << d << " | " << #e << ": " << e << " | " << #f << ": " << f << endl;




struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

GLFWwindow* window;
float blocks_vel = 0.01;
int score=0 , wrong_block = 0, over = 0, black=0 , red_coll = 0, green_coll=0, black_shot=0, mistargets=0;
double WINDOW_HEIGHT  = 900;
double WINDOW_WIDTH  = 900;

map<char, bool> mouse;
map<int, bool> kmap;

int fbwidth=WINDOW_HEIGHT, fbheight=WINDOW_HEIGHT;
void play_sound()
{

	//SOUND

	ao_device *device;
	ao_sample_format format;
	int default_driver;
	char *buffer;
	int buf_size;
	int sample;
	float freq = 444;

/* -- Initialize -- */
	

	/* -- Setup for default driver -- */

	default_driver = ao_default_driver_id();

	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 2;
	format.rate = 44100;
	format.byte_format = AO_FMT_LITTLE;

	/* -- Open driver -- */
	device = ao_open_live(default_driver, &format, NULL /* no options */);
	if (device == NULL) {
		fprintf(stderr, "Error opening device.\n");
		
	}

	/* -- Play some stuff -- */
	buf_size = format.bits/8 * format.channels * format.rate;
	buffer = new char[buf_size];

	for (int i = 0; i < format.rate; i++) {
		sample = (int)(0.75 * 32768.0 *
			sin(2 * M_PI * freq * ((float) i/format.rate)));

		/* Put the same stuff in left and right channel */
		buffer[4*i] = buffer[4*i+2] = sample & 0xff;
		buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
	}
	ao_play(device, buffer, buf_size);
	delete buffer;
	 /* -- Close and shutdown -- */
	ao_close(device);

	//END SOUND
}

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

 struct Sprite
 {
 	VAO* vao;
 	float x;
 	float y;
 	float width;
 	float height;
 	float speed;
 	bool status;
 	float angle;
 	string color;
 	GLfloat zoom;

 };



 Sprite bucket_red, bucket_green;
 Sprite bullet , canon, camera;
 vector<Sprite> blocks, mirrors, deflector;

 bool drag = false;
 float bullet_vel = 0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
 void reshapeWindow (GLFWwindow* window, int width, int height)
 {
 	
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
 	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

 	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
 	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
	   Matrices.projection = glm::ortho(-camera.zoom, camera.zoom, -camera.zoom, camera.zoom, 0.1f, 500.0f);
	}

// Creates the triangle object used in this sample code
	bool mouse_on_object(Sprite obj)
	{
		float r = 1;
		double mouse_x, mouse_y;
		double mouse_x_normalised, mouse_y_normalised;
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		mouse_x_normalised = (mouse_x*2*camera.zoom/fbwidth)-camera.zoom;
		mouse_y_normalised = camera.zoom-(mouse_y*2*camera.zoom/fbheight);
		
		if(pow(mouse_x_normalised-obj.x, 2 )+ pow(mouse_y_normalised-obj.y, 2 ) <= pow(r , 2))
		{
			return true;
		}
		return false;
	}


	void create_bucket_red()
	{
		static const GLfloat vertex_buffer_data [] = {
    -0.6,-1,0, // vertex 1
    0.6,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -0.6,-1,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {

	255.0/256, 128.0/256, 128.0/256,
	255.0/256, 128.0/256, 128.0/256,
	255.0/256, 128.0/256, 128.0/256,
	255.0/256, 128.0/256, 128.0/256,
	255.0/256, 128.0/256, 128.0/256,
	255.0/256, 128.0/256, 128.0/256,

};

bucket_red.color = "red";
bucket_red.x = -2;
bucket_red.y = -4;
bucket_red.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
bucket_red.width = 2.4;
bucket_red.height = 2;

}

void create_bucket_green()
{
	static const GLfloat vertex_buffer_data [] = {
     -0.6,-1,0, // vertex 1
    0.6,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -0.6,-1,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {
	187.0/256, 255.0/256, 153.0/256,
   187.0/256, 255.0/256, 153.0/256, // color 1
   187.0/256, 255.0/256, 153.0/256,
   187.0/256, 255.0/256, 153.0/256,
   187.0/256, 255.0/256, 153.0/256,
   187.0/256, 255.0/256, 153.0/256,
};

  // create3DObject creates and returns a handle to a VAO that can be used later
bucket_green.color = "green";
bucket_green.x = 2;
bucket_green.y = -4;
bucket_green.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
bucket_green.width = 2.4;
bucket_green.height = 2;
}
// Creates the rectangle object used in this sample code
void create_canon ()
{
	static const GLfloat vertex_buffer_data [] = {
	-0.4,-0.1,0, // vertex 1
	0.4,-0.1,0, // vertex 2
	0.4, 0.1,0, // vertex 3

	-0.4, -0.1, 0,
	-0.4, -0.3 , 0,
	-0.1 , -0.1, 0,

	-0.1, -0.1, 0,
	-0.1, -0.3, 0,
	-0.4, -0.3, 0,


	0.4, 0.1,0, // vertex 3
	-0.4, 0.1,0, // vertex 4
	-0.4,-0.1,0  // vertex 1

};

static const GLfloat color_buffer_data [] = {
  153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   

153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   153.0/255, 102.0/255, 51.0/255,
   

};
canon.vao = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
canon.x = -3.5;
canon.y = 0;

}

void create_bullet ()
{

	

	float r =  ((float)rand()) /  (RAND_MAX);
	float g =  ((float)rand()) /   (RAND_MAX);
	float b =  ((float)rand()) /  (RAND_MAX);

	static const GLfloat vertex_buffer_data [] = {
    -0.12,-0.1,0, // vertex 1
    0.12,-0.1,0, // vertex 2
    0.12, 0.1,0, // vertex 3

    0.12, 0.1,0, // vertex 3
    -0.12, 0.1,0, // vertex 4
    -0.12,-0.1,0  // vertex 1
};

 GLfloat color_buffer_data [] = {
   r,g,b, // color 1
    r,g,b, // color 2
    r,g,b, // color 3

    r,g,b, // color 3
   	r,g,b, // color 4
    r,g,b, // color 1
};

Sprite temp;
if(bullet.status == 0)
{
	temp.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	temp.angle = canon.angle;
	temp.y = canon.y;
	temp.x = canon.x;
	temp.width=0.24;
	temp.height=0.2;
	temp.status=1;
	bullet=temp;

	std::thread first (play_sound);
	first.detach();
}

}

void create_blocks()
{
	static const GLfloat vertex_buffer_data [] = {
    -0.12,-0.1,0, // vertex 1
    0.12,-0.1,0, // vertex 2
    0.12, 0.1,0, // vertex 3

    0.12, 0.1,0, // vertex 3
    -0.12, 0.1,0, // vertex 4
    -0.12,-0.1,0  // vertex 1
};

static const GLfloat color_buffer_data_red [] = {
   1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
   	1,0,0, // color 4
    1,0,0, // color 1
};

static const GLfloat color_buffer_data_green [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0,  // color 1
};

static const GLfloat color_buffer_data_black [] = {
   0,0, 0, // color 1
   0,0, 0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
   	0,0,0, // color 4
   0,0, 0 // color 1
};

Sprite temp;
float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
if(r<=0.33)
{
	temp.color = "red";
	temp.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_red, GL_FILL);
}
else if(r<=0.66)
{
	temp.color = "green";
	temp.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_green, GL_FILL);
}
else
{
	temp.color = "black";
	temp.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_black, GL_FILL);
}

int LO = -3.5;
int HI = 4;
float random = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
temp.x= random;
temp.y=4;
temp.width=0.24;
temp.height=0.2;
temp.status = 1;
blocks.push_back(temp);
if (blocks[0].status == 0)
{
	blocks.erase(blocks.begin());
}



}
float camera_rotation_angle = 90;
float rectangle_rotation = 0; 
float triangle_rotation = 0;

void create_mirrors ()
{
	static const GLfloat vertex_buffer_data [] = {
    -0.12,-0.1,0, // vertex 1
    0.12,-0.1,0, // vertex 2
    0.12, 0.1,0, // vertex 3

    0.12, 0.1,0, // vertex 3
    -0.12, 0.1,0, // vertex 4
    -0.12,-0.1,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,

	153.0/256, 255.0/256, 255.0/256,
};
Sprite temp;
temp.vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
int LO = -3.5;
int HI = 4;
float random = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
temp.x= random;
temp.y=4;
temp.width=0.24;
temp.height=0.2;
temp.status = 1;
LO = 45;
HI = 135;
random = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
temp.angle = random;
mirrors.push_back(temp);
if (mirrors[0].status == 0)
{
	mirrors.erase(mirrors.begin());
}

}

void create_deflector ()
{
	static const GLfloat vertex_buffer_data [] = {
    -0.7,-0.05,0, // vertex 1
    0.7,-0.05,0, // vertex 2
    0.7, 0.05,0, // vertex 3

    0.7, 0.05,0, // vertex 3
    -0.7, 0.05,0, // vertex 4
    -0.7,-0.05,0  // vertex 1
};
static const GLfloat color_buffer_data [] = {
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,
	153.0/256, 255.0/256, 255.0/256,

};

Sprite temp;
temp.vao =create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
temp.angle = 135;
temp.status = 1;
temp.x = 2;
temp.y = 2;
temp.height = 1.4;
deflector.push_back(temp);
temp.vao =create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
temp.x = 1;
temp.y = -0.5;
temp.angle = 120;
deflector.push_back(temp);

}

//Move canon according to mouse position
void mouse_cannon_direction(GLFWwindow* window)
{
	double mouse_x, mouse_y;
	glfwGetCursorPos(window,&mouse_x,&mouse_y);
	float angle=atan((fbheight /2 - canon.y *fbheight / (2*camera.zoom) - mouse_y)/mouse_x);
	canon.angle= angle * 180 / M_PI;
}


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
	glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0 ,0 , 3), glm::vec3(camera.x, camera.y, 0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  

  /* Render your scene */

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();

  // BUCKET_RED
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_bucket_red = glm::translate (glm::vec3(bucket_red.x, bucket_red.y, 0));        // glTranslatef
// rotate about vector (-1,1,1)
  Matrices.model *= (translate_bucket_red);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bucket_red.vao);
  for(int i=0 ; i<blocks.size(); i++)
  {
  	if(blocks[i].status == 0) continue;
  	// PUT COLLISION CONDITION
  	if(abs(blocks[i].y - bucket_red.y) <= (bucket_red.height+blocks[i].height)/2 && abs(blocks[i].x - bucket_red.x) <= (bucket_red.width+blocks[i].width)/2)
  	{   
  		if(blocks[i].color == "red")
  		{
  			score += 10;
  			cout<<"SCORE : "<<score<<endl;
  			red_coll++;
  			blocks[i].status = 0;
  		}
  		else if(blocks[i].color == "green")
  		{
  			score -= 5;
  			cout<<"SCORE : "<<score<<endl;
  			blocks[i].status = 0;
  			wrong_block++;
  		}
  		else if(blocks[i].color == "black")
  		{
  			
  			cout<<"SCORE : "<<score<<endl;
  			black++;
  			
  		}

  		if(black == 10)
  		{
  			cout<<"10 Black bricks collected!\n";
  			over = 1;
  		}
  		if(wrong_block == 20)
  		{
  			cout<<"Number of wrong blocks more than 10\n";
  			over = 1;
  		}


  		blocks[i].status = false;

  	}
  	else if(abs(blocks[i].y - bucket_green.y) <= (bucket_green.height+blocks[i].height)/2 && abs(blocks[i].x - bucket_green.x) <= (bucket_green.width+blocks[i].width)/2)
  	{   
  		if(blocks[i].color == "red")
  		{
  			score -= 5;
  			cout<<"SCORE : "<<score<<endl;

  			blocks[i].status = 0;
  		}
  		else if(blocks[i].color == "green")
  		{
  			score += 10;
  			green_coll++;
  			cout<<"SCORE : "<<score<<endl;

  			blocks[i].status = 0;
  			wrong_block++;
  		}
  		else if(blocks[i].color == "black")
  		{
  			
  			cout<<"SCORE : "<<score<<endl;
  			black++;
  			
  		}

  		if(black == 10)
  		{
  			cout<<"10 Black bricks collected!\n";
  			over = 1;
  		}

  		if(wrong_block == 20)
  		{
  			cout<<"Number of wrong blocks more than 10\n";
  			over = 1;
  		}
  		blocks[i].status = false;
  	}

  }

  //BUCKET_GREEN
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_bucket_green = glm::translate (glm::vec3(bucket_green.x, bucket_green.y, 0));        // glTranslatef
// rotate about vector (-1,1,1)
  Matrices.model *= (translate_bucket_green);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bucket_green.vao);


  //CANON
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_canon = glm::translate (glm::vec3(canon.x, canon.y, 0));
  glm::mat4 rotate_canon = glm::rotate((float)(canon.angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        // glTranslatef
// rotate about vector (-1,1,1)
  Matrices.model *= (translate_canon * rotate_canon);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(canon.vao);
  //camera_rotation_angle++; // Simulating camera rotation

  // MOUSE DRAG 
  //FOR CANON
  if(mouse['L'] == 1)
  {
  	if(mouse_on_object(canon))
  	{
  		drag = true;
  		// trace1("On canon");
  		double mouse_x, mouse_y;
  		double mouse_y_normalised, mouse_x_normalised;
  		glfwGetCursorPos(window, &mouse_x, &mouse_y);
  		mouse_x_normalised = (mouse_x*2*camera.zoom/fbwidth)-camera.zoom;
  		mouse_y_normalised = camera.zoom-(mouse_y*2*camera.zoom/fbheight);
  		canon.y = mouse_y_normalised;
  	}
  	if(mouse_on_object(bucket_red))
  	{
  		drag = true;
  		// trace1("On red bucket");
  		double mouse_x, mouse_y;
  		double mouse_y_normalised, mouse_x_normalised;
  		glfwGetCursorPos(window, &mouse_x, &mouse_y);
  		mouse_x_normalised = (mouse_x*2*camera.zoom/fbwidth)-camera.zoom;
  		mouse_y_normalised = camera.zoom-(mouse_y*2*camera.zoom/fbheight);
  		bucket_red.x = mouse_x_normalised;
  	}
  	if(mouse_on_object(bucket_green))
  	{
  		drag = true;
  		// trace1("On green bucket");
  		double mouse_x, mouse_y;
  		double mouse_y_normalised, mouse_x_normalised;
  		glfwGetCursorPos(window, &mouse_x, &mouse_y);
  		mouse_x_normalised = (mouse_x*2*camera.zoom/fbwidth)-camera.zoom;
  		mouse_y_normalised = camera.zoom-(mouse_y*2*camera.zoom/fbheight);
  		bucket_green.x = mouse_x_normalised;
  	}
  }

  if(mouse['R'] == 1)
  {
  	double mouse_x_old, mouse_y_old;
  	double mouse_y_new, mouse_x_new;
  	glfwGetCursorPos(window, &mouse_x_old, &mouse_y_old);
  	std::this_thread::sleep_for (std::chrono::milliseconds(50));
  	glfwGetCursorPos(window, &mouse_x_new, &mouse_y_new);
  	double change = mouse_x_new - mouse_x_old;
  	if(change<0)
  		camera.x -= 0.1 ;
  	else if(change >0)
  		camera.x+=0.1;


  }

  //KEYBOARD COMBOS
  if((kmap[GLFW_KEY_LEFT_CONTROL]==1 && kmap[GLFW_KEY_LEFT]==1)||((kmap[GLFW_KEY_RIGHT_CONTROL]==1 && kmap[GLFW_KEY_LEFT]==1)))
  {
  	bucket_red.x-=0.1;
  }
  else if((kmap[GLFW_KEY_LEFT_CONTROL]==1 && kmap[GLFW_KEY_RIGHT]==1)||((kmap[GLFW_KEY_RIGHT_CONTROL]==1 && kmap[GLFW_KEY_RIGHT]==1)))
  {
  	bucket_red.x+=0.1;
  }
  else if((kmap[GLFW_KEY_LEFT_ALT]==1 && kmap[GLFW_KEY_LEFT]==1)||((kmap[GLFW_KEY_RIGHT_ALT]==1 && kmap[GLFW_KEY_LEFT]==1)))
  {
  	bucket_green.x-=0.1;
  }
  else if((kmap[GLFW_KEY_LEFT_ALT]==1 && kmap[GLFW_KEY_RIGHT]==1)||((kmap[GLFW_KEY_RIGHT_ALT]==1 && kmap[GLFW_KEY_RIGHT]==1)))
  {
  	bucket_green.x+=0.1;
  }



  //BULLET
  if(bullet.status==1)
  {	
  	float bullet_vel=0.1;
  	if(bullet.x > 2*camera.zoom || bullet.y>camera.zoom || bullet.x <-2*camera.zoom || bullet.y <-camera.zoom)
  	{
  		bullet.x =0 ;
  		bullet.y = 0;
  		bullet.status = 0;
  	}
  	else
  	{
  		float w=0.2;
  		Matrices.model = glm::mat4(1.0f);


  		glm::mat4 translate_bullet = glm::translate (glm::vec3(bullet.x, bullet.y, 0));

        // glTranslatef
// rotate about vector (-1,1,1)
  		Matrices.model *= (translate_bullet);
  		MVP = VP * Matrices.model;
  		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  		draw3DObject(bullet.vao);
  		bullet.x += (bullet_vel)*cos(bullet.angle*M_PI/180.0f);
  		bullet.y += (bullet_vel)*sin(bullet.angle*M_PI/180.0f);
  		for(int i=0 ; i<blocks.size(); i++)
  		{
  			if(abs(bullet.y-blocks[i].y) < w && abs(bullet.x-blocks[i].x)<w && blocks[i].status == 1)
  			{
  				if(blocks[i].color != "black")
  				{
  					score -= 5;
  					mistargets++;
  				}  				
  				else
  				{
  					black_shot++;
  					score+=5;
  				}
  				blocks[i].status = 0;
  				bullet.x =0 ;
  				bullet.y =0;
  				bullet.status = 0;
  				break;
  			}
  		}

  		for(int i=0 ; i<deflector.size(); i++)
  		{
  			float m = tan(deflector[i].angle * M_PI / 180);
  			float c = deflector[i].y - m*deflector[i].x;
  			if(	(bullet.y- m*bullet.x)<c+0.1 && (bullet.y - m*bullet.x)>c-0.1 && bullet.y >= deflector[i].y-(deflector[i].height/2)*sin(deflector[i].angle*M_PI / 180) && bullet.y <= deflector[i].y + (deflector[i].height/2)*sin(deflector[i].angle*M_PI / 180))
  			{		 
  				bullet.angle =2*deflector[i].angle - bullet.angle ; 

  			}
  		}



  		// cout << bullet_x << " " << bullet_y << endl;

  		for(int i=0 ; i<mirrors.size(); i++)
  		{
  			if(abs(bullet.y-mirrors[i].y) < w && abs(bullet.x-mirrors[i].x)<w && mirrors[i].status == 1)
  			{
  				mirrors[i].status = 0;
  				bullet.angle =2*mirrors[i].angle - bullet.angle ; 
  				break;
  			}
  			
  		}
  	} 
  }

    //DEFLECTORS
  for(int i=0 ; i<deflector.size(); i++)
  {
  	Matrices.model = glm::mat4(1.0f);

  	glm::mat4 translate_deflectors = glm::translate (glm::vec3(deflector[i].x, deflector[i].y, 0));
  glm::mat4 rotate_deflectors = glm::rotate((float)(deflector[i].angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        // glTranslatef
// rotate about vector (-1,1,1)
  Matrices.model *= (translate_deflectors * rotate_deflectors);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(deflector[i].vao);

}

  //BLOCKS
int len  = blocks.size();
for(int i=0 ; i<len ; i++)
{
	if(blocks[i].status == 1 && blocks[i].y < -4 )
		blocks[i].status = 0 ;
	if (blocks[i].status == 1)
	{
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translate_blocks = glm::translate (glm::vec3(blocks[i].x , blocks[i].y, 0));
        // glTranslatef
// rotate about vector (-1,1,1)
		Matrices.model *= (translate_blocks);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(blocks[i].vao);
		blocks[i].y-=blocks_vel;
	}
}



  //MIRRORS

len  = mirrors.size();
for(int i=0;i<len;i++)
{
	if(mirrors[i].status == 1 && mirrors[i].y < -4 )
		mirrors[i].status = 0 ;
	if(mirrors[i].status == 0) continue;
	if(abs(mirrors[i].y  - bucket_red.y) <= (bucket_red.height+mirrors[i].height)/2 && abs(mirrors[i].x - bucket_red.x) <= (bucket_red.width+mirrors[i].width)/2)
	{
		mirrors[i].status=0;
	}
	else if(abs(mirrors[i].y - bucket_green.y) <= (bucket_green.height+mirrors[i].height)/2 && abs(mirrors[i].x - bucket_green.x) <= (bucket_green.width+mirrors[i].width)/2)
	{	
		mirrors[i].status=0;
	}
}
for(int i=0 ; i<len ; i++)
{
	if (mirrors[i].status == 1)
	{ 		

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translate_mirrors = glm::translate (glm::vec3(mirrors[i].x , mirrors[i].y, 0));

        // glTranslatef
// rotate about vector (-1,1,1)
  	glm::mat4 rotate_mirrors = glm::rotate((float)(mirrors[i].angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        // glTranslatef
// rotate about vector (-1,1,1)

  	Matrices.model *= (translate_mirrors * rotate_mirrors);
  	MVP = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  	
  // draw3DObject draws the VAO given to it using current MVP matrix
  	draw3DObject(mirrors[i].vao);
  	mirrors[i].y-=blocks_vel;

  }

}	


//TEXT ON SCREEN

// Create a pixmap font from a TrueType file.
// FTGLPixmapFont font("./Arial.ttf");

// If something went wrong, bail out.
// if(font.Error())
//     trace1("error");

// Set the font size and render a small text.
// font.FaceSize(72);
// font.Render("Hello World!");

}


void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		kmap[key] = 0;
		switch (key) {
			case GLFW_KEY_A:
			canon.angle+=5;
			break;
			case GLFW_KEY_D:
			canon.angle-=5;
			break;
			case GLFW_KEY_S:
			canon.y +=0.5;
			break;
			case GLFW_KEY_F:
			canon.y -=0.5;
			break;

			case GLFW_KEY_UP:
			camera.zoom +=0.5f;
			break;
			case GLFW_KEY_DOWN:
			camera.zoom -=0.5f;
			break;
			case GLFW_KEY_LEFT:
			if(kmap[GLFW_KEY_LEFT_CONTROL] ==0 && kmap[GLFW_KEY_LEFT_ALT]==0 && kmap[GLFW_KEY_RIGHT_CONTROL] ==0 && kmap[GLFW_KEY_RIGHT_ALT]==0)
				camera.x -=0.5;
			break;
			case GLFW_KEY_RIGHT	:
			if(kmap[GLFW_KEY_LEFT_CONTROL] ==0 && kmap[GLFW_KEY_LEFT_ALT]==0 && kmap[GLFW_KEY_RIGHT_CONTROL] ==0 && kmap[GLFW_KEY_RIGHT_ALT]==0)
				camera.x +=0.5;
			break;
			case GLFW_KEY_N	:
			if(blocks_vel<0.1)
				blocks_vel += 0.01;
			break;
			case GLFW_KEY_M :
			if(blocks_vel>0.01)
				blocks_vel -= 0.01;
			break;

			case GLFW_KEY_SPACE:
			create_bullet();
			break;
			default:
			break;
		}
	}
	else if (action == GLFW_PRESS) {
		kmap[key] = 1;
		switch (key) {
			case GLFW_KEY_ESCAPE:
			quit(window);
			break;
			default:
			break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
		quit(window);
		break;
		default:
		break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_PRESS)
		{
			mouse['L'] = 1;	
		}
		else if (action == GLFW_RELEASE)
		{
			mouse['L'] = 0;
			if(drag == false)
			{
				mouse_cannon_direction(window);
				create_bullet();
			}
			drag = false;
		}

		break;
		case GLFW_MOUSE_BUTTON_RIGHT:
		
		if (action == GLFW_PRESS)
		{
			mouse['R'] = 1;	
		}
		else if (action == GLFW_RELEASE)
		{
			mouse['R'] = 0;
		}
		
		break;
		default:
		break;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset == 1)
		camera.zoom-=0.5f;
	else if(yoffset == -1)
		camera.zoom+=0.5f;
}


/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Brick Breaker Pro", NULL, NULL);

    if (!window) {
    	glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	create_deflector ();	
	create_bucket_red (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	create_bucket_green ();
	create_canon ();
	camera.x=0;
	camera.y=0;
	camera.zoom = 4.0f;
	
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f, 204.0/256, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}


int main (int argc, char** argv)
{
	srand (static_cast <unsigned> (time(0)));
	int width = 900;
	int height = 900;
	ao_initialize();


	

	window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;


    /* Draw in loop */

	while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
		
		if(over == 1) 
		{
			cout<<"\n\n=========================================================================\n\n";
			cout<<"GAME OVER!"<<endl;
			cout<<"Green collected in green bucket : "<<green_coll<<endl;
			cout<<"Red collected in red bucket : "<<red_coll<<endl;
			cout<<"Number of rightly fired shots : "<<black_shot<<endl;
			cout<<"Number of wrongly fired shots : "<<mistargets<<endl;
			cout<<"SCORE : "<<score<<endl;
			cout<<"\n\n=========================================================================\n\n";
			break;
		}
			draw();
			reshapeWindow(window, width, height);
        // Swap Frame Buffer in double buffering
			glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
			glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
        	create_blocks();
        	create_mirrors();
        	last_update_time = current_time;
        }
    }
    
// Delete the text's VBO, the shader and the texture
    glfwTerminate();
    ao_shutdown();
    exit(EXIT_SUCCESS);
}
