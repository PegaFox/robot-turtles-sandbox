#ifndef ROBOT_TURTLES_SANDBOX_TILE_HPP
#define ROBOT_TURTLES_SANDBOX_TILE_HPP

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <glm/vec2.hpp>
#include <glm/fwd.hpp>

class Tile
{
  public:
    enum Type
    {
      None,
      Turtle,
      Gem,
      Crate,
      StoneWall,
      IceWall
    };

    enum Color
    {
      Green = 0,
      Blue = 1,
      Pink = 2,
      Red = 3,
      BlankGreen = 4,
      BlankBlue = 5,
      BlankPink = 6,
      BlankRed = 7
    };

    static sf::Texture texture;
    static const int spriteSize = 68;

    Tile(Type type = None, Color color = Green): type(type), color(color)
    {
      sprite.setOrigin(sf::Vector2f(spriteSize*0.5f, spriteSize*0.5f));
      updateSprite();
    }

    Color getColor() const
    {
      return color;
    }

    void setColor(Color color)
    {
      this->color = color;
      updateSprite();
    }

    uint8_t getDir() const
    {
      return sprite.getRotation().asDegrees()/90;
    }

    void setDir(uint8_t dir)
    {
      sprite.setRotation(sf::degrees(dir*90));
    }

    Type getType() const
    {
      return type;
    }

    void setType(Type type)
    {
      this->type = type;
      updateSprite();
    }

    void draw(sf::RenderWindow& SCREEN, glm::i16vec2 pos)
    {
      if (type != None)
      {
        sf::Vector2f sfPos(pos.x * spriteSize+spriteSize*0.5f, pos.y * spriteSize+spriteSize*0.5f);
        sprite.setPosition(sfPos);
        SCREEN.draw(sprite);
      }
    }
  protected:
    void updateSprite()
    {
      sprite.setTextureRect(sf::IntRect(sf::Vector2i((type < StoneWall ? (type-1)*spriteSize*4 + (color & 3)*spriteSize : spriteSize*8 + type*spriteSize), /*((color & 4) >> 2)*spriteSize*/0), sf::Vector2i(spriteSize, spriteSize)));
    }

    sf::Sprite sprite = sf::Sprite(texture);

    Type type;

    Color color;
};
sf::Texture Tile::texture;

#endif // ROBOT_TURTLES_SANDBOX_TILE_HPP
