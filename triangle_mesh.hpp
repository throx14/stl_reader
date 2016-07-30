/*
 triangle_mesh.hpp
 GSOC16 ViennaMesh Puzzle
 Alexander Toifl, 2016
*/


#ifndef TRIANGLE_MESS_HPP_
#define TRIANGLE_MESS_HPP_

#include <string>
#include <vector>
#include <fstream>


/*
Every mesh described by STL file format is made of triangles only.
Thus, a TriangleMesh consists of Triangles, which are characterized by veritices (Point).
*/


/*
Point simultaneously represents vertices and vectors (in particular facet normals) in cartesian coordinates. Declared as struct to emphasize that coordinates can be directly accessed
without getter or setter functions.
*/
struct Point
{

    double x, y, z;

    const std::string str_rep() const; //returns string representation (eq. Point(1, 2.3, 1))

    //componentwise addition and subtraction
    Point operator+(Point p);
    Point operator-(Point p);

    //componentwise comparison
    bool operator==(Point p);
    bool operator!=(Point p);

};

Point cross_product(const Point p1, const Point p2); //calculates cross product in cartesian coordinates
Point unit_vector(const Point p); //calculates unit vector (norm = 1)

double norm(const Point p); //calculates euclidean norm

/*
A triangle is defined by three vertices and a facet normal. The facet normal should be provided by the STL file but to ensure that
right handed orientation is applied, the facet normal relying on the given vertices can be calculated too.
*/
class Triangle
{
public:
    Triangle();

    void add_vertex(Point vertex); //in total only three vertices can be added
    void clear_vertices(); //vector of vertices is erased.
    const std::vector<Point>& get_vertices() const;

     /* directly set facet normal. If Point normal is not normalized, it carried through before setting.
     It is not checked, whether normal suits to the contained vertices.*/
    void set_facet_normal(Point normal);
    const Point& get_facet_normal() const;

    Point calculate_facet_normal(); //using right hand rule, result is normalized.

    //check if triangle contains three vertices and if facet normal is correctly oriented
    bool check_triangle() ;

private:
    std::vector<Point> vertices;
    Point facet_normal; //is ensured to be normalized
};


/*
A TriangleMesh is characterized by a name and a list of Triangles. Additionally, the number of triangle is stored (triangle_count).
*/
class TriangleMesh
{
public:
    TriangleMesh();
    const std::string str_rep() const; //get string representation
    void add_triangle(Triangle tri);

    void set_name(std::string n);
    std::string get_name() const;

    long get_triangle_count() const;

private:
    std::vector<Triangle> triangles;
    long triangle_count;
    std::string name;


};

/*
STLParser manages file handling (by using the concept of RAII even in exception case filehandle is closed) and STL ACSII file parsing.
To be correctly parsed the given STL file has to be stricly in accordance with the format given in
people.sc.fsu.edu/~jburkardt/data/stla/stla.html
In particular, the correct facet unit vector has to be provided. Negative coordinates are possible. Only ONE solid can be defined, all further
are ignored.
*/
class STLParser
{
    public:
        explicit STLParser(std::string filename);
         ~STLParser(); //deconstructor ensures filehandle closing even when exceptions occur
         void to_triangle_mesh(TriangleMesh& mesh); //

    private:
        std::ifstream file;
        std::string content; //file content with addional newlines in order to enable tokenization in to_triangle_mesh().
        std::vector<std::string> tokens;

        //The STL keywords (as 'solid', 'facet', ..., 'endfacet', 'endsolid') define states during the parsing process.
        enum {OUTSIDE = 0, SOLID, FACET, NORMAL, NORMAL_COORD, OUTER, LOOP, END} state;

        //reads three consecutive floating point numbers to point. A priori it is checked if all number actually exist.
        Point parse_coordinates(unsigned long token_cnt);

        void read_content();
};


#endif
