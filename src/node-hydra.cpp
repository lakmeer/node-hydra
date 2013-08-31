#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <node.h>
#include <v8.h>

#include "sixense/sixense.h"


using namespace std;
using namespace v8;


// Reference Constants

const int PO2[11] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };

const float epsilon = 0.0000001;
const float PI      = 3.1415926535;

const float qT[5] = { 0, 0.25, 0.5, 0.75, 1 };
const float oT[8] = {
    0.0625,
    0.0625 + 0.125,
    0.0625 + 0.125 * 2,
    0.0625 + 0.125 * 3,
    0.0625 + 0.125 * 4,
    0.0625 + 0.125 * 5,
    0.0625 + 0.125 * 6,
    0.0625 + 0.125 * 7
};


// For Timing

clock_t     now, then;
long double elapsed;


// Module Shared Reusable State

sixenseAllControllerData allControllerData;
sixenseControllerData controllerData;

int  sequenceNum;
bool hasBeenInitialised = false;




// Helpers

bool bitAt ( unsigned int byte, int bit ) {
  return (bool)( ( byte & PO2[bit] ) >> bit );
}

float epsilonCrossing ( float b ) {
  return ( b < epsilon || b > 1 - epsilon ) ? 0 : b;
}

int toQuadrant ( float b, float r ) {
  if ( r == 0 ) { return 0; }
  return (b >= qT[0] && b < qT[1]) ? 1 :
         (b >= qT[1] && b < qT[2]) ? 2 :
         (b >= qT[2] && b < qT[3]) ? 3 : 4;
}

int toOctant ( float b, float r ) {
  if ( r == 0 ) { return 0; }
  return (b > oT[0] && b < oT[1]) ? 2 :
         (b > oT[1] && b < oT[2]) ? 3 :
         (b > oT[2] && b < oT[3]) ? 4 :
         (b > oT[3] && b < oT[4]) ? 5 :
         (b > oT[4] && b < oT[5]) ? 6 :
         (b > oT[5] && b < oT[6]) ? 7 :
         (b > oT[6] && b < oT[7]) ? 8 : 1;
}



// Start hydra, wait for 1 base station and 2 controllers to connect

Handle<Value> Init( const Arguments& args ) {
  HandleScope scope;

  int init = sixenseInit();

  if ( init > 0 ) {
    ThrowException(Exception::Error(String::New("Hydra init failed. Check connection.")));
  }

  while ( !sixenseIsBaseConnected(0) ) { usleep( 1000 ); }
  while ( sixenseGetNumActiveControllers() < 2 ) { usleep( 100000 ); }

  sixenseSetActiveBase(0);
  hasBeenInitialised = true;

  return scope.Close( Number::New( init ) );
}

// Shut down hydra

Handle<Value> Exit( const Arguments& args ) {
  HandleScope scope;
  int res;

  if ( !hasBeenInitialised ) {
    ThrowException(Exception::Error(String::New("No need to exit - Hyrda not initialised.")));
  } else {
    res = sixenseExit();
  }

  return scope.Close( Number::New( res ) );
}

// Get controller data

Handle<Value> Update( const Arguments& args ) {
  HandleScope scope;

  // Get time since last update
  now     = clock();
  elapsed = (long double)(((long double)now - (long double)then) * 1000 / CLOCKS_PER_SEC);
  then    = now;

  if ( !hasBeenInitialised ) {
    ThrowException(Exception::Error(String::New("Hydra not initialised yet.")));
  }

  // Get raw data from Hydra
  sixenseGetAllNewestData( &allControllerData );

  // Set up scaffolding of new JS object
  Local<Object> acd         = Object::New();
  Local<Object> controllers = Array::New();

  // Process Controllers individually
  for ( int i_c = 0; i_c < sixenseGetNumActiveControllers(); i_c++ ) {

    // Extract CD from ACD
    controllerData = allControllerData.controllers[i_c];

    // Create new JS object to represent controller
    Local<Object> controller  = Object::New();


    //
    // Record sequence number
    //

    // Both controllers should have the same seqnum, so subsequent controller
    // data should be able to overwrite the previous one. We write it to the
    // top level here so we can export it with the unified ACD object later on.

    sequenceNum = (int)controllerData.sequence_number;


    //
    // Process controller data
    //

    // Position/Time Derivatives

    Local<Object> js_pos = Array::New();
    for (int i_pos = 0; i_pos < 3; i_pos++ ) {
      js_pos->Set( Number::New(i_pos), Number::New( controllerData.pos[i_pos] )  );
    }
    controller->Set( String::NewSymbol("position"), js_pos  );


    // Rotation Matrix
    // Rotation Quaternion

    Local<Object> rot = Array::New();
    for (int i_rot = 0; i_rot < 4; i_rot++ ) {
      rot->Set( Number::New(i_rot), Number::New( controllerData.rot_quat[i_rot] ));
    }
    controller->Set( String::NewSymbol("rotation"), rot );


    // Joystick

    Local<Object> joystick = Object::New();

    // Cartesian
    float x = controllerData.joystick_x;
    float y = controllerData.joystick_y;

    // Polar
    float r = sqrt( x * x + y * y );
    float t = atan2(y, x);

    // Videogame-style - quad and oct are zero if stick is neutral
    float b  = epsilonCrossing( ( atan2(-x, -y) + PI ) / (2 * PI) );
    int quad = toQuadrant(b, r);
    int oct  = toOctant(b, r);

    joystick->Set( String::NewSymbol("x"), Number::New( x ));
    joystick->Set( String::NewSymbol("y"), Number::New( y ));
    joystick->Set( String::NewSymbol("r"), Number::New( r ));
    joystick->Set( String::NewSymbol("θ"), Number::New( t ));

    joystick->Set( String::NewSymbol("bearing"),  Number::New( b ));
    joystick->Set( String::NewSymbol("quadrant"), Number::New( quad ));
    joystick->Set( String::NewSymbol("octant"),   Number::New( oct  ));

    controller->Set( String::NewSymbol("joystick"), joystick );


    // Trigger

    controller->Set( String::NewSymbol("trigger"), Number::New( controllerData.trigger ));


    // Buttons

    Local<Object> buttons = Object::New();

    buttons->Set( String::NewSymbol("button1"),  Boolean::New( bitAt( controllerData.buttons, 5 )));
    buttons->Set( String::NewSymbol("button2"),  Boolean::New( bitAt( controllerData.buttons, 6 )));
    buttons->Set( String::NewSymbol("button3"),  Boolean::New( bitAt( controllerData.buttons, 3 )));
    buttons->Set( String::NewSymbol("button4"),  Boolean::New( bitAt( controllerData.buttons, 4 )));

    buttons->Set( String::NewSymbol("home"),     Boolean::New( bitAt( controllerData.buttons, 0 )));
    buttons->Set( String::NewSymbol("bumper"),   Boolean::New( bitAt( controllerData.buttons, 7 )));
    buttons->Set( String::NewSymbol("joystick"), Boolean::New( bitAt( controllerData.buttons, 8 )));

    buttons->Set( String::NewSymbol("bitmask"),  Number::New( controllerData.buttons ));

    controller->Set( String::NewSymbol("buttons"), buttons );


    // Attach newly genereated controller data to JS ACD object
    controllers->Set( Number::New(i_c), controller );
  }

  // Calculate separation
  Local<Object> separation = Array::New();

  float sep[3];

  for (int i_sep = 0; i_sep < 3; i_sep++) {
    sep[i_sep] =
      allControllerData.controllers[0].pos[i_sep] -
        allControllerData.controllers[1].pos[i_sep];
    separation->Set( Number::New(i_sep), Number::New(sep[i_sep]));
  }

  separation->Set( Number::New( 3 ),
    Number::New(
      sqrt( sep[0] * sep[0] + sep[1] * sep[1] + sep[2] * sep[2])
    )
  );

  // Add top-level data to ACD object
  acd->Set( String::NewSymbol("sequenceNum"), Number::New( sequenceNum ) );
  acd->Set( String::NewSymbol("elapsedTime"), Number::New( elapsed ) );
  acd->Set( String::NewSymbol("controllers"), controllers );
  acd->Set( String::NewSymbol("separation"),  separation );

  // Return ACD object to Node
  return scope.Close( acd );
}


void ModuleBind( Handle<Object> exports ) {

  exports->Set(String::NewSymbol("update"), FunctionTemplate::New( Update )->GetFunction());
  exports->Set(String::NewSymbol("init"),   FunctionTemplate::New( Init   )->GetFunction());
  exports->Set(String::NewSymbol("exit"),   FunctionTemplate::New( Exit   )->GetFunction());

}

NODE_MODULE(hydra, ModuleBind)
