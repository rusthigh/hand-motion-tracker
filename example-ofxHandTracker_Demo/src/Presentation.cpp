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

	ofDirectory dir = ofDirectory("pptx/");  
	dir.listDir(); // populates filelist
	//vector<ofFile> files = dir.getFiles();

    for (int i=0; i<dir.numFiles(); i++) {
		ofImage currentImage;

		currentImage.loadImage(dir.getPath(i));
        images.push_back(currentImage);
              
        if (maxHeight < currentImage.height) {
            maxHeight = currentImage.height;
        }            
        if (maxWidth < currentImage.width) {
            maxWidth = currentImage.width;
        }
    }
    
        
    sumWidth = maxWidth * images.size();
        
    galleryArea.allocate(maxWidth, maxHeight);
    gal