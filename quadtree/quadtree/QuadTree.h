#include <SFML/System.hpp>
#include <forward_list>

struct Point
{
	float x, y;
	
	Point(float x = 0, float y = 0) :
		x(x), y(y)
	{
	}
	
	Point(const Point& p) :
		x(p.x), y(p.y)
	{
	}
	
	bool operator==(const Point& p) const
	{
		return (x == p.x) && (y == p.y);
	}
	
	void operator=(const Point& p)
	{
		x = p.x;
		y = p.y;
	}
};

struct Rect
{
	float x, y, w, h;
	
	Rect(float x = 0, float y = 0, float w = 0, float h = 0) :
		x(x), y(y),
		w(w), h(h)
	{
	}
	
	Rect(const Point& position, const Point& size) :
		x(position.x), y(position.y),
		w(size.x), h(size.y)
	{
	}
	
	Rect(const Rect& r) :
		x(r.x), y(r.y),
		w(r.w), h(r.h)
	{
	}
	
	bool Contains(float px, float py) const
	{
		return sf::FloatRect(x, y, w, h).contains(px, py);
	}
	
	bool Contains(const Point& p) const
	{
		return Contains(p.x, p.y);
	}

	bool Intersects(const Rect& r) const
	{
		const float interLeft   = std::max(std::min(x, x + w), std::min(r.x, r.x + r.w));
		const float interTop    = std::max(std::min(y, y + h), std::min(r.y, r.y + r.h));
		const float interRight  = std::min(std::max(x, x + w), std::max(r.x, r.x + r.w));
		const float interBottom = std::min(std::max(y, y + h), std::max(r.y, r.y + r.h));
		
		return (interLeft < interRight) && (interTop < interBottom);
	}
};

class QuadTree
{
	const Rect rect;
	const std::size_t maxPoints;
	std::size_t pointCount;
	QuadTree** quads;
	Point* points;
public:
	QuadTree(const Rect& rect, std::size_t maxPoints = 4) :
		rect(rect),
		maxPoints(maxPoints),
		pointCount(0),
		points(new Point[maxPoints])
	{}

	~QuadTree()
	{
		delete [] points;

		if(quads)
		{
			for(std::size_t i = 0; i < 4; i++)
				delete quads[i];
			delete [] quads;
		}
	}

	std::size_t GetMaxPoints() const { return maxPoints; }
	std::size_t GetPointCount() const { return pointCount; }
	const Rect& GetRect() const { return rect; }

	bool Add(const Point& newPoint)
	{
		if(!rect.Contains(newPoint))
			return false;

		if(pointCount < maxPoints)
		{
			points[pointCount++] = newPoint;
			return true;
		}
		else
		{
			if(!quads)
				InitQuads();

			for(std::size_t i = 0; i < 4; i++)
			{
				if(quads[i]->Add(newPoint))
					return true;
			}
		}

		return false;
	}

	bool Contains(const Point& point)
	{
		for(std::size_t i = 0; i < pointCount; i++)
		{
			if(point == points[i])
				return true;
		}

		if(quads)
		{
			for(std::size_t i = 0; i < 4; i++)
			{
				if(quads[i]->Contains(point))
					return true;
			}
		}

		return false;
	}

	void GetPoints(std::forward_list<Point>& list)
	{
		for(std::size_t i = 0; i < pointCount; i++)
		{
			list.push_front(points[i]);
		}
	}

	void GetFromRect(const Rect& subRect, std::forward_list<Point>& list)
	{
		if (!rect.Intersects(subRect))
			return;

		for(std::size_t i = 0; i < pointCount; i++)
		{
			if(subRect.Contains(points[i]))
				list.push_front(points[i]);
		}

		if(quads)
		{
			for(std::size_t i = 0; i < 4; i++)
			{
				quads[i]->GetFromRect(subRect, list);
			}
		}	
	}

	std::size_t CountQuads()
	{
		if(quads)
			return quads[0]->CountQuads() + quads[1]->CountQuads() + quads[2]->CountQuads() + quads[3]->CountQuads();
		return 1;
	}

	QuadTree** GetQuads()
	{
		return quads;
	}

private:
	void InitQuads()
	{
		quads = new QuadTree*[4];

		const float w = rect.w / 2.f;
		const float h = rect.h / 2.f;

		quads[0] = new QuadTree(Rect(rect.x,	 rect.y,	 w, h), maxPoints);
		quads[1] = new QuadTree(Rect(rect.x + w, rect.y,	 w, h), maxPoints);
		quads[2] = new QuadTree(Rect(rect.x + w, rect.y + h, w, h), maxPoints);
		quads[3] = new QuadTree(Rect(rect.x,	 rect.y + h, w, h), maxPoints);
	}
};
