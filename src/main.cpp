#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    //ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

    ofGLWindowSettings settings;
    settings.width = 1200;
    settings.height = 1200;
    settings.setGLVersion(3,3);
    ofCreateWindow(settings);
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
