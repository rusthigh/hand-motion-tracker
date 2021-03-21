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
    galleryImages.allocate(sumWidth, maxHeight);
    imageIndex = 0;
        
    imagePos = ofPoint(0, 0);
    imageNextPos = imagePos;
    isSliding = false;
        
    galleryImages.begin();
    ofClear(0,0,0,0);
    for (int i=0; i<images.size(); i++) {
        images[i].draw((maxWidth * i) + maxWidth/2 - images[i].width/2, maxHeight/2 - images[i].height/2, images[i].width, images[i].height);
    }
    galleryImages.e