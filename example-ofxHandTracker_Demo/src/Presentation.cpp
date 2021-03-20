//
//  Presentation.cpp
//  BSystem?
//
//  Created by Blaz
//
//

#include "Presentation.h"

Presentation::Presentation() {
    setup();
}

Presentation::Presentation(string _assetsFilename, string _menusFilename, string _vizualType) {
    setup();
}
 
void Presentation::setup() {

    images.clear();
        
    int sumWidth = 0;
    int maxHeight = 0;
    int maxWidth = 0;

	ofDirectory dir = ofDi