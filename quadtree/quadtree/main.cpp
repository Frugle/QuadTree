
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <ctime>

#include "QuadTree.h"

void AddRandomPoints(QuadTree& qt, QuadTree& qt2, unsigned int numberOfPoints)
{
	const Rect& rect = qt.GetRect();
	for(int i = 0; i < numberOfPoints; i++)
	{
		float x = rect.x + ((float)rand() / RAND_MAX) * rect.w;
		float y = rect.y + ((float)rand() / RAND_MAX) * rect.h;
		
		bool success = qt.Add(Point(x, y));

		if (!success)
		{
			i--;
			continue;
		}

		qt2.Add(Point(x, y));
	}


}

float vector2fLength(const sf::Vector2f &v)
{
	return std::fabsf(std::sqrtf(std::powf(v.x, 2.f) + std::powf(v.y, 2.f)));
}

QuadTree *qt;
QuadTree *qtLinear;
sf::RenderWindow *rw;
sf::RenderTexture *rt;
const float margin = 20.0f;

sf::Font* font;

sf::Text* nText;
sf::Text* nValue;

sf::Text* nSelectedText;
sf::Text* nSelectedValue;

sf::Text* qtTimeText;
sf::Text* qtTimeValue;

sf::Text* qtLinearTimeText;
sf::Text* qtLinearTimeValue;

float qtTime;

void initQt(int pointCount = 0, int pointsPerQuad = 100)
{
	qt = new QuadTree(Rect(0, 0, 10000, 10000), pointsPerQuad);
	qtLinear = new QuadTree(Rect(0, 0, 10000, 10000), 10000000);
	
	srand(1);
	AddRandomPoints(*qt, *qtLinear, pointCount);
	std::cout << "qt.CountQuads() = " << qt->CountQuads() << std::endl;
}

void initRender(sf::Vector2i reso = sf::Vector2i(1200, 900))
{
	rw = new sf::RenderWindow(sf::VideoMode(reso.x, reso.y), "Quadtree", sf::Style::Close);
	rt = new sf::RenderTexture();

	rt->create(
		rw->getSize().y - 2 * margin, 
		rw->getSize().y - 2 * margin);

	font = new sf::Font();
	font->loadFromFile("DejaVuSans.ttf");

	const unsigned int fontSize = 36u * ((reso.x - reso.y) / 300.0f);
	const float startY = margin;
	const float textMargin = fontSize * 1.05f;

	nText = new sf::Text("N", *font, fontSize);
	nText->setColor(sf::Color(160, 160, 160));
	nText->setPosition(2 * margin + rt->getSize().x, startY);
	nValue = new sf::Text("", *font, fontSize);
	nValue->setPosition(2 * margin + rt->getSize().x, startY + textMargin);

	nSelectedText = new sf::Text("Valittujen N", *font, fontSize);
	nSelectedText->setColor(sf::Color(160, 160, 160));
	nSelectedText->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 3.0f);
	nSelectedValue = new sf::Text("N/A", *font, fontSize);
	nSelectedValue->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 4.0f);

	qtTimeText = new sf::Text("Nelipuu-haku", *font, fontSize);
	qtTimeText->setColor(sf::Color(160, 160, 160));
	qtTimeText->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 6.0f);
	qtTimeValue = new sf::Text("N/A", *font, fontSize);
	qtTimeValue->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 7.0f);

	qtLinearTimeText = new sf::Text("Peräkkäishaku", *font, fontSize);
	qtLinearTimeText->setColor(sf::Color(160, 160, 160));
	qtLinearTimeText->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 9.0f);
	qtLinearTimeValue = new sf::Text("N/A", *font, fontSize);
	qtLinearTimeValue->setPosition(2 * margin + rt->getSize().x, startY + textMargin * 10.0f);
}

int drawPoints(sf::RenderTarget &target, const Rect rect, const sf::Color color)
{
	sf::Vector2u targetSize = target.getSize();
	Rect rootRect = qt->GetRect();
	
	sf::Vector2f scale(
		targetSize.x / rootRect.w,
		targetSize.y / rootRect.h);

	// Quadtree points
	std::forward_list<Point> points;

	int count = 0;

	qt->GetFromRect(rect, points);

	sf::CircleShape circle(1.5f, 6u);
	for(std::forward_list<Point>::iterator i = points.begin(); i != points.end(); i++)
	{
		sf::Vector2f point((*i).x * scale.x, (*i).y * scale.y);
		circle.setPosition(point);
		circle.setFillColor(color);

		target.draw(circle);

		count++;
	}

	return count;
}

sf::Time perfFindPoints(const Rect& rect, QuadTree* theQt)
{
	std::forward_list<Point> points;

	const int samples = 50;
	const float samplesf = samples;

	sf::Time time = sf::Time();
	sf::Clock perfClock;
	for (int i = 0; i < samples; i++)
	{
		perfClock.restart();
		theQt->GetFromRect(rect, points);
		time += perfClock.getElapsedTime();
		points.clear();
	}

	return (time / samplesf);
};

void drawQt(sf::RenderTarget &target, QuadTree* qt, QuadTree *root)
{
	sf::Vector2u targetSize = target.getSize();

	QuadTree *current = qt;

	Rect rect = current->GetRect();
	Rect rootRect = root->GetRect();
	
	sf::Vector2f scale(
		targetSize.x / rootRect.w,
		targetSize.y / rootRect.h);

	// Quadtree rect
	sf::RectangleShape shape(sf::Vector2f(rect.w * scale.x, rect.h * scale.y));
	shape.setPosition(
		rect.x / rootRect.w * targetSize.x, 
		rect.y / rootRect.h * targetSize.y);

	shape.setFillColor(sf::Color::Transparent);
	shape.setOutlineColor(sf::Color(194, 60, 158));
	shape.setOutlineThickness(-1.0f);
	
	target.draw(shape);

	if (current->GetQuads())
	{
		for(int i = 0; i < 4; i++)
		{
			drawQt(target, (current->GetQuads())[i], root);
		}
	}
}

bool changedRt = true;
bool changedRw = true;
Rect* selectRect = 0;
Rect* qtRect = 0;

void render()
{
	if (changedRt)
	{
		rt->clear();

		int count;

		// Render Quadtree to RenderTexture
		drawQt(*rt, qt, qt);
		count = drawPoints(*rt, qt->GetRect(), sf::Color(56, 198, 226));

		std::stringstream ss;
		ss << count;
		nValue->setString(ss.str());

		if (qtRect)
		{
			count = drawPoints(*rt, *qtRect, sf::Color(255, 231, 11));

			std::stringstream ss;
			ss << count;
			nSelectedValue->setString(ss.str());
		}
		else
		{
			nSelectedValue->setString("0");
		}

		rt->display();
	}

	if (changedRw || changedRt)
	{
		// Render RenderTexture to RenderWindow
		rw->clear();
		sf::Sprite sprite(rt->getTexture());
		sprite.setPosition(margin, margin);
		rw->draw(sprite);

		if (selectRect)
		{
			sf::RectangleShape shape(sf::Vector2f(selectRect->w, selectRect->h));
			shape.setPosition(selectRect->x, selectRect->y);

			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineColor(sf::Color(255, 119, 67));
			shape.setOutlineThickness(-1.0f);
	
			(*rw).draw(shape);
		}

		(*rw).draw(*nText);
		(*rw).draw(*nValue);

		(*rw).draw(*nSelectedText);
		(*rw).draw(*nSelectedValue);

		(*rw).draw(*qtTimeText);
		(*rw).draw(*qtTimeValue);

		(*rw).draw(*qtLinearTimeText);
		(*rw).draw(*qtLinearTimeValue);

		rw->display();
	}
}

bool mouseDown = false;
sf::Vector2f downPos;
sf::Vector2f upPos;

void handleMouseDown()
{
	mouseDown = true;
	downPos = rw->mapPixelToCoords(sf::Mouse::getPosition(*rw));
}

void handleMouseUp()
{
	mouseDown = false;
	upPos = rw->mapPixelToCoords(sf::Mouse::getPosition(*rw));

	sf::Vector2f marginAddPos = upPos - sf::Vector2f(margin, margin);

	sf::Vector2u targetSize = rt->getSize();
	Rect rootRect = qt->GetRect();
	sf::Vector2f scale(
		targetSize.x / rootRect.w,
		targetSize.y / rootRect.h);

	sf::Vector2f qtPos(marginAddPos.x / scale.x, marginAddPos.y / scale.y);

	if (selectRect)
	{
		if (qtRect)
		{
			delete qtRect;
			qtRect = 0;
		}

		qtRect = new Rect(
			(selectRect->x - margin) / scale.x, 
			(selectRect->y - margin) / scale.y, 
			selectRect->w / scale.x, 
			selectRect->h / scale.y);

		std::stringstream ss;
		ss << (perfFindPoints(*qtRect, qt).asSeconds() * 1000.0f) << "ms";
		qtTimeValue->setString(ss.str());

		ss.str("");
		ss << (perfFindPoints(*qtRect, qtLinear).asSeconds() * 1000.0f) << "ms";
		qtLinearTimeValue->setString(ss.str());

		if (selectRect)
		{
			delete selectRect;
			selectRect = 0;
		}

		changedRt = true;
	}
	else
	{
		if (qtRect)
		{
			delete qtRect;
			qtRect = 0;
		}

		if (selectRect)
		{
			delete selectRect;
			selectRect = 0;
		}

		Point point(qtPos.x, qtPos.y);

		if (qt->GetRect().Contains(point) && qt->Add(point))
		{
			qtLinear->Add(point);
			changedRt = true;
		}
	}
}

void handleMouseMove()
{
	sf::Vector2f curPos = rw->mapPixelToCoords(sf::Mouse::getPosition(*rw));

	if (mouseDown && vector2fLength(downPos - curPos) > 10.f)
	{
		if (selectRect)
		{
			delete selectRect;
			selectRect = 0;
		}

		selectRect = new Rect(downPos.x, downPos.y, curPos.x - downPos.x, curPos.y - downPos.y);

		changedRw = true;
	}
}

void handleKeyUp(sf::Keyboard::Key key)
{
	if (key == sf::Keyboard::Key::Num1)
		AddRandomPoints(*qt, *qtLinear, 10);
	else if (key == sf::Keyboard::Key::Num2)
		AddRandomPoints(*qt, *qtLinear, 100);
	else if (key == sf::Keyboard::Key::Num3)
		AddRandomPoints(*qt, *qtLinear, 1000);
	else if (key == sf::Keyboard::Key::Num4)
		AddRandomPoints(*qt, *qtLinear, 10000);
	else if (key == sf::Keyboard::Key::Num5)
		AddRandomPoints(*qt, *qtLinear, 100000);
	else if (key == sf::Keyboard::Key::Num6)
		AddRandomPoints(*qt, *qtLinear, 1000000);
	else if (key == sf::Keyboard::Key::Num0)
	{
		delete qt;
		delete qtLinear;

		initQt(0, 100);
	}

	changedRt = true;
}

void run()
{
	while (rw->isOpen())
    {
        sf::Event event;
		
        while (rw->pollEvent(event))
        {
			switch (event.type)
			{
				case sf::Event::Closed:
				{
					rw->close();
					break;
				}
				case sf::Event::MouseMoved:
				{
					handleMouseMove();
					break;
				}
				case sf::Event::MouseButtonPressed:
				{
					handleMouseDown();
					break;
				}
				case sf::Event::MouseButtonReleased:
				{
					handleMouseUp();
					break;
				}
				case sf::Event::KeyReleased:
				{
					handleKeyUp(event.key.code);
					break;
				}
				case sf::Event::GainedFocus:
				{
					changedRt = true;
					break;
				}
				default:
					break;
			}
        }

		if (changedRt || changedRw)
		{
			render();

			changedRt = false;
			changedRw = false;
		}

		sf::sleep(sf::milliseconds(10));
    }
}

// Launch parameters
// 1	Reso X
// 2	Reso Y
// 3	N
// 4	PointsPerQuad
// 5	Performance test area top left		X
// 6	Performance test area top left		Y
// 7	Performance test area bottom right	X
// 8	Performance test area bottom right	Y

// STDOUT output (after "OUTPUT" is printed)
// qtSpeed\tlinearSpeed

int main(int argc, char* argv[])
{
	//srand(1);

	std::cout << "Param count: " << argc << std::endl;
	for (int i = 0; i < argc; i++)
	{
		std::cout << atoi(argv[i]) << std::endl;
	}

	std::cout << std::endl;

	if (argc == 3)
		initQt(0, 100);
	else if (argc == 4)
		initQt(atoi(argv[3]), 100);
	else if (argc == 5)
		initQt(atoi(argv[3]), atoi(argv[4]));
	else if (argc > 5)
	{
		initQt(atoi(argv[3]), atoi(argv[4]));

		std::cout << "OUTPUT" << std::endl;

		float left = atoi(argv[5]);
		float top = atoi(argv[6]);
		float right = atoi(argv[7]);
		float bottom = atoi(argv[8]);

		Rect rect(left, top, right - left, bottom - top);

		float qtTime = perfFindPoints(rect, qt).asSeconds() * 1000.0f;
		float linearTime = perfFindPoints(rect, qtLinear).asSeconds() * 1000.0f;

		printf("%10.3f", qtTime);
		printf("\t");
		printf("%10.3f", linearTime);

		return 0;
	}
	else
		initQt(0, 100);

	if (argc > 2)
	{
		float resX = atoi(argv[1]);
		float resY = atoi(argv[2]);

		if (resX != 0 && resY != 0)
		{
			initRender(sf::Vector2i(resX, resY));
		}
	}
	else
	{
		initRender();
	}

	run();

	delete qt;
	delete qtLinear;
	delete rt;
	delete rw;

	return 0;
}


