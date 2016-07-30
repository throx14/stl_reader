/*
 puzzle_cpp
 GSOC16 ViennaMesh Puzzle
 Alexander Toifl, 2016
*/


#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#include <string>
#include <iterator>

#include <math.h>

#include "triangle_mesh.hpp"

#define SUCCESS 0

/*
Comments regarding design and data structure are given in header file 'triangle_mesh.hpp'.

Exception/Error handling. For this puzzle exception handles has been simplified in the sense that every
kind of error should result in a termination of parsing and priting of the cause. Thus, only runtime_errors are used.
*/


/*As facet unit normal vectors have to given in stl file, small numeric inaccuracies are tolerated.*/
const double EPS = 0.001;

using namespace std;

//struct Point function implementations
const string Point::str_rep() const
{
    stringstream repStream;
    repStream <<  "Point(" <<  x << ", " << y << ", " << z << ")";
    return repStream.str();
}

Point Point::operator+(Point p)
{
    return Point {x + p.x, y + p.y, z + p.z};
}
Point Point::operator-(Point p)
{
    return Point {x - p.x, y - p.y, z - p.z};
}

bool Point::operator==(Point p)
{
    return ( (fabs(p.x - x) < EPS) && (fabs(p.y - y ) < EPS) && (fabs(p.z - z) < EPS));

}

bool Point::operator!=(Point p)
{
    return !(*this == p);

}

//static function
Point cross_product(const Point p, const Point q)
{
    return Point {p.y * q.z - p.z * q.y, p.z * q.x - p.x * q.z, p.x * q.y - p.y * q.x};
}

//static function
double norm(const Point p)
{
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

//static function
Point unit_vector(const Point p)
{
    double pnorm = norm(p);

    //check if norm is zero
    if(fabs(pnorm) < EPS)
    {
        throw runtime_error("Point::unit_vector: tried to normalized Point(0, 0, 0)");
    }

    return Point {p.x / pnorm, p.y / pnorm, p.z / pnorm};
}

//class Triangle function implementations

Triangle::Triangle()
{
    vertices = vector<Point>();
    facet_normal = Point {0,0,0};
}

void Triangle::add_vertex(Point vertex)
{
    if(vertices.size() < 3) //not more than 3 vertices allowed for a triangle.
        vertices.push_back(vertex);
    else
        throw runtime_error {"Triangle::add_vertex: Tried to add fourth vertex of triangle."};
}


void Triangle::clear_vertices()
{
    vertices.clear();
}

const vector<Point>& Triangle::get_vertices() const
{
    return vertices;
}

void Triangle::set_facet_normal(Point normal)
{
    //check if facet normal to set is unit vector
    if( (1.0 - fabs(norm(normal))) < EPS)
    {
        facet_normal = normal;
    }//if not, normalize it
    else
    {
        facet_normal = unit_vector(normal);
    }

}

const Point& Triangle::get_facet_normal() const
{
    return facet_normal;
}

Point Triangle::calculate_facet_normal()
{
    if(vertices.size() == 3)
    {

        Point vec1 = vertices[1] - vertices[0]; //vertex_0 ------> vertex_1
        Point vec2 = vertices[2] - vertices[0]; //vertex_0 ------> vertex_2

        return unit_vector(cross_product(vec1, vec2)); //order of arguments ensures right handed orientation.
    }

    else
        throw runtime_error {"Triangle::calculate_facet_normal(): Not enough vertices associated with triangle."};
}

bool Triangle::check_triangle()
{
    //check if triangle contains three vertices
    if(vertices.size() != 3)
    {
        cerr << "check_triangle(): Number of vertices is not equal to 3.\n";
        return false;
    }

    //calculate facet normal based on the vertices and compare to facet_normal
    if(calculate_facet_normal() != facet_normal)
    {
        cerr << "check_triangle(): calculated and given facet normals do not match.\n";
        cerr << "calulated facet normal: " << calculate_facet_normal().str_rep() << endl;
        cerr << "given facet normal: " << facet_normal.str_rep() << endl;
        return false;
    }

    return true;
}

//class TriangleMesh function implementations

TriangleMesh::TriangleMesh()
{
    triangle_count = 0;
    triangles = vector<Triangle>();
}


const string TriangleMesh::str_rep() const
{
    stringstream repStream;

    repStream << "Triangle Mesh: " << name << "\n";

    for(Triangle t : triangles)
    {
        repStream << "\tTriangle\n";

        for(Point p: t.get_vertices())
        {
            repStream << "\t\t" << p.str_rep() << "\n";
        }
    }

    repStream << endl;

    return repStream.str();
}

void TriangleMesh::add_triangle(Triangle tri)
{
    if(tri.check_triangle() == true) //triangle is only added to mesh, if it contains three vertices and the facet normal is valid
    {
        triangles.push_back(tri);
        triangle_count++;
    }
    else
        throw runtime_error {"Triangle::add_triagle: tried to add an invalid triangle (triangle check failed)"};

}

void TriangleMesh::set_name(string n)
{
    name = n;
}

string TriangleMesh::get_name() const
{
    return name;
}

 long TriangleMesh::get_triangle_count() const
 {
     return triangle_count;
 }


//impementation of class STLParser functions

STLParser::STLParser(string filename)
{
    file.open(filename); //closed by constructor

    if(file.fail() != 0)
    {
        string message = "Filehandle::FileHandle: Could not read file '" + filename + "'";
        throw runtime_error {message};
    }


    read_content();

}

STLParser::~STLParser()
{
    file.close(); //implementation of RAII concept
}

void STLParser::read_content()
{
    string line {};
    content = "";

    while (getline(file,line, ' '))
    {
        content.append(line);
        content.append("\n");
    }
}


Point STLParser::parse_coordinates(unsigned long token_cnt)
{
    if(token_cnt + 2 < tokens.size()) //check if three further tokens exist (all coordinates)
    {
        try
        {
            double coordx = stod(tokens[token_cnt]);
            double coordy = stod(tokens[token_cnt + 1]);
            double coordz = stod(tokens[token_cnt + 2]);

            return Point {coordx, coordy, coordz};

        }
        catch(invalid_argument& e)
        {
            throw runtime_error("STLParser::to_triangle_mesh: file format error - could not parse coordinates");
        }
    }
    else
        throw runtime_error("STLParser::to_triangle_mesh: file format error - could not parse coordinates");

}

void STLParser::to_triangle_mesh(TriangleMesh& mesh)
{
    //tokenize content string into vector of strings
    istringstream iss(content);
    tokens = {istream_iterator<string>{iss},
              istream_iterator<string>{}
             };

    TriangleMesh mesh_tmp {}; //temporary mesh

    Triangle tri {}; //temporary triangle

    /*
    Parsing is carried through by the concept that the STL syntax implies keywords (as 'solid', 'facet'), which define sections that
    are mostly ended by 'end'+state.
    While the tokens are iterated over starting form the beginning, the keywords change the parsing state (described by a enumeration, see header file).
    The present implementation requires the given STL file to be stricly of the format described in people.sc.fsu.edu/~jburkardt/data/stla/stla.html .
    Even missing a 'endfacet' or 'endsolid' leads to a parsing fail.

    The following code may seems lengthy, but essentially every STL file format error should recognizable.
    */

    state = OUTSIDE;
    unsigned long cnt = 0;
    while(cnt < tokens.size())
    {
        switch(state)
        {
        case OUTSIDE:
            if(tokens[cnt] == "solid")
            {
                if(cnt + 1 < tokens.size() )//after keyword solid there should be a string respresenting the solid's name
                {
                    if(tokens[cnt+1] == "facet") //if not, the mesh is titled "no name"
                    {
                        mesh_tmp.set_name("no name");
                    }
                    else
                    {
                        mesh_tmp.set_name(tokens[cnt + 1]);
                        cnt = cnt + 1;
                    }
                    state = SOLID; //now we are inside the solid segment
                }
                else
                    throw runtime_error("STLParser::to_triangle_mesh: file format error - could not parse solid's name");
            }
            break;

        case SOLID:
            if(tokens[cnt] == "facet")
                state = FACET;
            else if(tokens[cnt] == "endsolid")
                state = END;
            else
                throw runtime_error("STLParser::to_triangle_mesh: keyword 'facet' or keyword 'endsolid' expected, but not found.");

            break;

        case FACET:
            if(tokens[cnt] == "normal") //the facet section is introduced by two keyword 'facet normal', thus two states are required
                state = NORMAL;
            else if(tokens[cnt] == "endfacet")
                state = SOLID;
            else
                throw runtime_error("STLParser::to_triangle_mesh: file format error - double keyword 'facet normal' is not complete");
            break;

        case NORMAL:
            try //read facet normal
            {
                Point facet_normal = parse_coordinates(cnt);
                tri.set_facet_normal(facet_normal);
                state = NORMAL_COORD;
                cnt = cnt+2;
            }
            catch(runtime_error& e)
            {
                throw;
            }

            break;

        case NORMAL_COORD:
            if(tokens[cnt] == "outer")
                state = OUTER;
            else
                throw runtime_error("STLParser::to_triangle_mesh: file format error - keyword 'outer loop' is not complete or missing");
            break;

        case OUTER:
            if(tokens[cnt] == "loop")
                state = LOOP;
            else
                throw runtime_error("STLParser::to_triangle_mesh: file format error - keyword 'outer loop' is not complete or missing");
            break;

        case LOOP:
            if(tokens[cnt] == "vertex")
            {
                try //read vertex
                {
                    Point vertex = parse_coordinates(cnt+1);
                    tri.add_vertex(vertex);
                    cnt = cnt+3;
                }
                catch(runtime_error& e)
                {
                    throw;
                }
            }
            else if(tokens[cnt] == "endloop")
            {
                if (tri.check_triangle() == true)
                {
                    mesh_tmp.add_triangle(tri);
                    tri.clear_vertices();
                    state = FACET;
                }
                else
                {
                    throw runtime_error("STLParser::to_triangle_mesh: file format error - tried to create invalid triangle.");
                }
            }
            else
                throw runtime_error("STLParser::to_triangle_mesh: file format error - invalid 'outer loop' segment");

            break;


        case END:
            cnt = tokens.size(); //end parsing, all further solids are ignored
            break;
        }
        cnt++;
    }

    if(state != END)
    {
        throw runtime_error("STLParser::to_triangle_mesh: file format error - solid segment was never ended");
    }

    mesh = std::move(mesh_tmp);

}


int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        try
        {
            STLParser parser {argv[1]};
            TriangleMesh mesh {};


            //parsing STL file, please keep in mind that even a single uncorrect facet normal (right handed!) leads to a parsing fail.
            parser.to_triangle_mesh(mesh);


            cout << mesh.str_rep(); //resulting mesh is given in a textual style
            cout << "number of triangles = " << mesh.get_triangle_count() << endl; //addionally, the number of read triangles is printed

        }
        catch(const runtime_error& e)//every error is treated by printing the cause and quit the program.
        {
            cerr << "Error while parsing file '" << argv[1] << "':\n" << e.what() << endl;
        }
    }
    else
    {
        cout << "One argument required: STL (ASCII) filename\n";
    }



//    try
//    {
//        TriangleMesh mesh {};
//
//        Triangle tri1 {};
//
//        Point v1 {0,0,0};
//        Point v2 {1,0,0};
//        Point v3 {0,0,1};
//
//
//        tri1.add_vertex(v1);
//        tri1.add_vertex(v2);
//        tri1.add_vertex(v3);
//
//        tri1.set_facet_normal(Point {0, -1, 0});
//
//        Triangle tri2 {};
//
//
//
//        Point v4 {1, 0, 0};
//        Point v5 {0, 1, 0};
//        Point v6 {0, 0, 1};
//
//        //Point v7{1, 0, 0};
//
//        //DEBUGcout << "test: " << (Point{1,0,0} == Point{1,0,0});
//
//        tri2.add_vertex(v4);
//        tri2.add_vertex(v5);
//        tri2.add_vertex(v6);
//
//        //DEBUG cout << "facet normal = " << tri2.calculate_facet_normal().str_rep();
//
//        tri2.set_facet_normal(Point {0.577, 0.577, 0.577});
//
//        mesh.add_triangle(tri1);
//        mesh.add_triangle(tri2);
//
//        cout << mesh.str_rep();
//
//    }
//    catch (const MeshException& e)
//    {
//        cerr << e.get_description() << endl;
//    }


    return SUCCESS;
}
