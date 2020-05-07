/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
// Interactive tool that displays iris images with boundaries overlaid.

#include "irisBoundary.h"
#include <SFML/Graphics.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace sf;
using namespace std;

struct BoundaryPoint
{
   string hash,
          type;

   int x,
       y;
};

istream& operator>>(istream& is, BoundaryPoint& pt) {
   return is >> pt.hash >> pt.x >> pt.y >> pt.type;
}

int main(int argc, char* argv[])
{
   const string keys = "{help |        | print this message }"
                       "{@i   | <none> | image path         }"
                       "{@x   | <none> | pupil x coordinate }"
                       "{@y   | <none> | pupil y coordinate }"
                       "{@r   | <none> | pupil radius       }"
                       "{@R   | <none> | limbus radius      }";

   cv::CommandLineParser parser(argc, argv, keys);

   // Print help information.
   if (parser.has("help")) {
      parser.printMessage();
      return EXIT_SUCCESS;
   }

   // Check that parameters were actually specified.
   if (!parser.check()) {
      parser.printErrors();
      return EXIT_FAILURE;
   }

   const int x = parser.get<int>("@x"),
             y = parser.get<int>("@y"),
             r = parser.get<int>("@r"),
             R = parser.get<int>("@R");

   // Get paths to polar image.
   const string imgPath = parser.get<string>("@i");

   // Load a sprite to display.
   Texture texture;
   if (!texture.loadFromFile(imgPath)) {
      cerr << "Error loading image: " << imgPath << endl;
      return EXIT_FAILURE;
   }

   IrisBoundary pupil(IrisBoundary::Type::Pupil,   x, y, r, r),
                limbus(IrisBoundary::Type::Limbus, x, y, R, R);

   // Create the main window
   RenderWindow window(VideoMode(640, 480), "Iris Window");

   // Start the game loop
   while (window.isOpen())
   {
      // Process events
      Event event;

      // Did user make an exist request.
      while (window.pollEvent(event))
         if (Keyboard::isKeyPressed(Keyboard::Enter) || event.type == Event::Closed) {
            window.close();
         } else if (Keyboard::isKeyPressed(Keyboard::Left)) {
            --pupil.x;
            --limbus.x;
         } else if (Keyboard::isKeyPressed(Keyboard::Right)) {
            ++pupil.x;
            ++limbus.x;
         } else if (Keyboard::isKeyPressed(Keyboard::Up)) {
            --pupil.y;
            --limbus.y;
         } else if (Keyboard::isKeyPressed(Keyboard::Down)) {
            ++pupil.y;
            ++limbus.y;
         } else if (Keyboard::isKeyPressed(Keyboard::A)) {
            --pupil.a;
            --pupil.b;
         } else if (Keyboard::isKeyPressed(Keyboard::S)) {
            ++pupil.a;
            ++pupil.b;
         } else if (Keyboard::isKeyPressed(Keyboard::Q)) {
            --limbus.a;
            --limbus.b;
         } else if (Keyboard::isKeyPressed(Keyboard::W)) {
            ++limbus.a;
            ++limbus.b;
         }

      Sprite sprite(texture);

      // Clear screen
      window.clear();

      // Draw the sprite
      window.draw(sprite);

      CircleShape circle(pupil.a);

      // Set circle properties.
      circle.setFillColor(Color::Transparent);
      circle.setOutlineThickness(1);
      circle.setOutlineColor(Color::Red);
      circle.setOrigin(pupil.a - pupil.x, pupil.a - pupil.y);

      // Draw the pupil boundary.
      window.draw(circle);

      circle.setOutlineColor(Color::Yellow);
      circle.setOrigin(limbus.a - limbus.x, limbus.a - limbus.y);
      circle.setRadius(limbus.a);

      // Draw the limbus boundary.
      window.draw(circle);

      // Update the window
      window.display();
   }

   // Output results.
   cout << imgPath << " " << pupil.x << " " << pupil.y << " " << pupil.a << " " << limbus.a << endl;

   return EXIT_SUCCESS;
}

