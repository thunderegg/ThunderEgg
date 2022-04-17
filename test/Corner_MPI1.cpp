/***************************************************************************
 *  ThunderEgg, a library for solvers on adaptively refined block-structured
 *  Cartesian grids.
 *
 *  Copyright (c) 2021      Scott Aiton
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ***************************************************************************/
#include <ThunderEgg/Face.h>

#include <sstream>

#include <doctest.h>

using namespace std;
using namespace ThunderEgg;
using namespace ThunderEgg::tpl;

TEST_CASE("Corner<2> num_corners")
{
  CHECK(Corner<2>::number_of == 4);
}
TEST_CASE("Corner<3> num_corners")
{
  CHECK(Corner<3>::number_of == 8);
}

TEST_CASE("Corner<2> dimensionality")
{
  CHECK(Corner<2>::dimensionality == 0);
}
TEST_CASE("Corner<3> dimensionality")
{
  CHECK(Corner<3>::dimensionality == 0);
}

TEST_CASE("Corner<2> unsigned char constructor works")
{
  Corner<2> o(13);
  CHECK(o.getIndex() == 13);
}
TEST_CASE("Corner<3> unsigned char constructor works")
{
  Corner<3> o(13);
  CHECK(o.getIndex() == 13);
}
TEST_CASE("Corner<2> Default constructor works")
{
  Corner<2> o;
  CHECK(o == Corner<2>::null());
}
TEST_CASE("Corner<3> Default constructor works")
{
  Corner<3> o;
  CHECK(o == Corner<3>::null());
}
TEST_CASE("Corner<2> named constructors give expected index values")
{
  CHECK(Corner<2>::sw().getIndex() == 0);
  CHECK(Corner<2>::se().getIndex() == 1);
  CHECK(Corner<2>::nw().getIndex() == 2);
  CHECK(Corner<2>::ne().getIndex() == 3);
  CHECK(Corner<2>::null().getIndex() == 4);
}
TEST_CASE("Corner<3> named constructors give expected index values")
{
  CHECK(Corner<3>::bsw().getIndex() == 0);
  CHECK(Corner<3>::bse().getIndex() == 1);
  CHECK(Corner<3>::bnw().getIndex() == 2);
  CHECK(Corner<3>::bne().getIndex() == 3);
  CHECK(Corner<3>::tsw().getIndex() == 4);
  CHECK(Corner<3>::tse().getIndex() == 5);
  CHECK(Corner<3>::tnw().getIndex() == 6);
  CHECK(Corner<3>::tne().getIndex() == 7);
  CHECK(Corner<3>::null().getIndex() == 8);
}
TEST_CASE("Corner<2> opposite")
{
  CHECK(Corner<2>::sw().opposite() == Corner<2>::ne());
  CHECK(Corner<2>::se().opposite() == Corner<2>::nw());
  CHECK(Corner<2>::nw().opposite() == Corner<2>::se());
  CHECK(Corner<2>::ne().opposite() == Corner<2>::sw());
}
TEST_CASE("Corner<3> opposite")
{
  CHECK(Corner<3>::bsw().opposite() == Corner<3>::tne());
  CHECK(Corner<3>::bse().opposite() == Corner<3>::tnw());
  CHECK(Corner<3>::bnw().opposite() == Corner<3>::tse());
  CHECK(Corner<3>::bne().opposite() == Corner<3>::tsw());
  CHECK(Corner<3>::tsw().opposite() == Corner<3>::bne());
  CHECK(Corner<3>::tse().opposite() == Corner<3>::bnw());
  CHECK(Corner<3>::tnw().opposite() == Corner<3>::bse());
  CHECK(Corner<3>::tne().opposite() == Corner<3>::bsw());
}
TEST_CASE("Corner<2> getSides is as expected")
{
  {
    auto array = Corner<2>::sw().getSides();
    CHECK(array[0] == Side<2>::west());
    CHECK(array[1] == Side<2>::south());
  }
  {
    auto array = Corner<2>::se().getSides();
    CHECK(array[0] == Side<2>::east());
    CHECK(array[1] == Side<2>::south());
  }
  {
    auto array = Corner<2>::nw().getSides();
    CHECK(array[0] == Side<2>::west());
    CHECK(array[1] == Side<2>::north());
  }
  {
    auto array = Corner<2>::ne().getSides();
    CHECK(array[0] == Side<2>::east());
    CHECK(array[1] == Side<2>::north());
  }
}
TEST_CASE("Corner<3> getSides is as expected")
{
  {
    auto array = Corner<3>::bsw().getSides();
    CHECK(array[0] == Side<3>::west());
    CHECK(array[1] == Side<3>::south());
    CHECK(array[2] == Side<3>::bottom());
  }
  {
    auto array = Corner<3>::bse().getSides();
    CHECK(array[0] == Side<3>::east());
    CHECK(array[1] == Side<3>::south());
    CHECK(array[2] == Side<3>::bottom());
  }
  {
    auto array = Corner<3>::bnw().getSides();
    CHECK(array[0] == Side<3>::west());
    CHECK(array[1] == Side<3>::north());
    CHECK(array[2] == Side<3>::bottom());
  }
  {
    auto array = Corner<3>::bne().getSides();
    CHECK(array[0] == Side<3>::east());
    CHECK(array[1] == Side<3>::north());
    CHECK(array[2] == Side<3>::bottom());
  }
  {
    auto array = Corner<3>::tsw().getSides();
    CHECK(array[0] == Side<3>::west());
    CHECK(array[1] == Side<3>::south());
    CHECK(array[2] == Side<3>::top());
  }
  {
    auto array = Corner<3>::tse().getSides();
    CHECK(array[0] == Side<3>::east());
    CHECK(array[1] == Side<3>::south());
    CHECK(array[2] == Side<3>::top());
  }
  {
    auto array = Corner<3>::tnw().getSides();
    CHECK(array[0] == Side<3>::west());
    CHECK(array[1] == Side<3>::north());
    CHECK(array[2] == Side<3>::top());
  }
  {
    auto array = Corner<3>::tne().getSides();
    CHECK(array[0] == Side<3>::east());
    CHECK(array[1] == Side<3>::north());
    CHECK(array[2] == Side<3>::top());
  }
}
TEST_CASE("Corner<2> ==")
{
  CHECK(Corner<2>::sw() == Corner<2>::sw());
  CHECK_FALSE(Corner<2>::sw() == Corner<2>::se());
  CHECK_FALSE(Corner<2>::sw() == Corner<2>::nw());
  CHECK_FALSE(Corner<2>::sw() == Corner<2>::ne());
  CHECK_FALSE(Corner<2>::sw() == Corner<2>::null());

  CHECK_FALSE(Corner<2>::se() == Corner<2>::sw());
  CHECK(Corner<2>::se() == Corner<2>::se());
  CHECK_FALSE(Corner<2>::se() == Corner<2>::nw());
  CHECK_FALSE(Corner<2>::se() == Corner<2>::ne());
  CHECK_FALSE(Corner<2>::se() == Corner<2>::null());

  CHECK_FALSE(Corner<2>::nw() == Corner<2>::sw());
  CHECK_FALSE(Corner<2>::nw() == Corner<2>::se());
  CHECK(Corner<2>::nw() == Corner<2>::nw());
  CHECK_FALSE(Corner<2>::nw() == Corner<2>::ne());
  CHECK_FALSE(Corner<2>::nw() == Corner<2>::null());

  CHECK_FALSE(Corner<2>::ne() == Corner<2>::sw());
  CHECK_FALSE(Corner<2>::ne() == Corner<2>::se());
  CHECK_FALSE(Corner<2>::ne() == Corner<2>::nw());
  CHECK(Corner<2>::ne() == Corner<2>::ne());
  CHECK_FALSE(Corner<2>::ne() == Corner<2>::null());

  CHECK_FALSE(Corner<2>::null() == Corner<2>::sw());
  CHECK_FALSE(Corner<2>::null() == Corner<2>::se());
  CHECK_FALSE(Corner<2>::null() == Corner<2>::nw());
  CHECK_FALSE(Corner<2>::null() == Corner<2>::ne());
  CHECK(Corner<2>::null() == Corner<2>::null());
}
TEST_CASE("Corner<3> ==")
{
  CHECK(Corner<3>::bsw() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::bsw() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::bse() == Corner<3>::bsw());
  CHECK(Corner<3>::bse() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::bse() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::bse());
  CHECK(Corner<3>::bnw() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::bnw() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::bne() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::bnw());
  CHECK(Corner<3>::bne() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::bne() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::bne());
  CHECK(Corner<3>::tsw() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::tsw() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::tse() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::tsw());
  CHECK(Corner<3>::tse() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::tse() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::tse());
  CHECK(Corner<3>::tnw() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::tnw() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::tne() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::tnw());
  CHECK(Corner<3>::tne() == Corner<3>::tne());
  CHECK_FALSE(Corner<3>::tne() == Corner<3>::null());

  CHECK_FALSE(Corner<3>::null() == Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::bse());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::bne());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::tse());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::null() == Corner<3>::tne());
  CHECK(Corner<3>::null() == Corner<3>::null());
}
TEST_CASE("Corner<2> !=")
{
  CHECK_FALSE(Corner<2>::sw() != Corner<2>::sw());
  CHECK(Corner<2>::sw() != Corner<2>::se());
  CHECK(Corner<2>::sw() != Corner<2>::nw());
  CHECK(Corner<2>::sw() != Corner<2>::ne());
  CHECK(Corner<2>::sw() != Corner<2>::null());

  CHECK(Corner<2>::se() != Corner<2>::sw());
  CHECK_FALSE(Corner<2>::se() != Corner<2>::se());
  CHECK(Corner<2>::se() != Corner<2>::nw());
  CHECK(Corner<2>::se() != Corner<2>::ne());
  CHECK(Corner<2>::se() != Corner<2>::null());

  CHECK(Corner<2>::nw() != Corner<2>::sw());
  CHECK(Corner<2>::nw() != Corner<2>::se());
  CHECK_FALSE(Corner<2>::nw() != Corner<2>::nw());
  CHECK(Corner<2>::nw() != Corner<2>::ne());
  CHECK(Corner<2>::nw() != Corner<2>::null());

  CHECK(Corner<2>::ne() != Corner<2>::sw());
  CHECK(Corner<2>::ne() != Corner<2>::se());
  CHECK(Corner<2>::ne() != Corner<2>::nw());
  CHECK_FALSE(Corner<2>::ne() != Corner<2>::ne());
  CHECK(Corner<2>::ne() != Corner<2>::null());

  CHECK(Corner<2>::null() != Corner<2>::sw());
  CHECK(Corner<2>::null() != Corner<2>::se());
  CHECK(Corner<2>::null() != Corner<2>::nw());
  CHECK(Corner<2>::null() != Corner<2>::ne());
  CHECK_FALSE(Corner<2>::null() != Corner<2>::null());
}
TEST_CASE("Corner<3> !=")
{
  CHECK_FALSE(Corner<3>::bsw() != Corner<3>::bsw());
  CHECK(Corner<3>::bsw() != Corner<3>::bse());
  CHECK(Corner<3>::bsw() != Corner<3>::bnw());
  CHECK(Corner<3>::bsw() != Corner<3>::bne());
  CHECK(Corner<3>::bsw() != Corner<3>::tsw());
  CHECK(Corner<3>::bsw() != Corner<3>::tse());
  CHECK(Corner<3>::bsw() != Corner<3>::tnw());
  CHECK(Corner<3>::bsw() != Corner<3>::tne());
  CHECK(Corner<3>::bsw() != Corner<3>::null());

  CHECK(Corner<3>::bse() != Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bse() != Corner<3>::bse());
  CHECK(Corner<3>::bse() != Corner<3>::bnw());
  CHECK(Corner<3>::bse() != Corner<3>::bne());
  CHECK(Corner<3>::bse() != Corner<3>::tsw());
  CHECK(Corner<3>::bse() != Corner<3>::tse());
  CHECK(Corner<3>::bse() != Corner<3>::tnw());
  CHECK(Corner<3>::bse() != Corner<3>::tne());
  CHECK(Corner<3>::bse() != Corner<3>::null());

  CHECK(Corner<3>::bnw() != Corner<3>::bsw());
  CHECK(Corner<3>::bnw() != Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bnw() != Corner<3>::bnw());
  CHECK(Corner<3>::bnw() != Corner<3>::bne());
  CHECK(Corner<3>::bnw() != Corner<3>::tsw());
  CHECK(Corner<3>::bnw() != Corner<3>::tse());
  CHECK(Corner<3>::bnw() != Corner<3>::tnw());
  CHECK(Corner<3>::bnw() != Corner<3>::tne());
  CHECK(Corner<3>::bnw() != Corner<3>::null());

  CHECK(Corner<3>::bne() != Corner<3>::bsw());
  CHECK(Corner<3>::bne() != Corner<3>::bse());
  CHECK(Corner<3>::bne() != Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::bne() != Corner<3>::bne());
  CHECK(Corner<3>::bne() != Corner<3>::tsw());
  CHECK(Corner<3>::bne() != Corner<3>::tse());
  CHECK(Corner<3>::bne() != Corner<3>::tnw());
  CHECK(Corner<3>::bne() != Corner<3>::tne());
  CHECK(Corner<3>::bne() != Corner<3>::null());

  CHECK(Corner<3>::tsw() != Corner<3>::bsw());
  CHECK(Corner<3>::tsw() != Corner<3>::bse());
  CHECK(Corner<3>::tsw() != Corner<3>::bnw());
  CHECK(Corner<3>::tsw() != Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tsw() != Corner<3>::tsw());
  CHECK(Corner<3>::tsw() != Corner<3>::tse());
  CHECK(Corner<3>::tsw() != Corner<3>::tnw());
  CHECK(Corner<3>::tsw() != Corner<3>::tne());
  CHECK(Corner<3>::tsw() != Corner<3>::null());

  CHECK(Corner<3>::tse() != Corner<3>::bsw());
  CHECK(Corner<3>::tse() != Corner<3>::bse());
  CHECK(Corner<3>::tse() != Corner<3>::bnw());
  CHECK(Corner<3>::tse() != Corner<3>::bne());
  CHECK(Corner<3>::tse() != Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tse() != Corner<3>::tse());
  CHECK(Corner<3>::tse() != Corner<3>::tnw());
  CHECK(Corner<3>::tse() != Corner<3>::tne());
  CHECK(Corner<3>::tse() != Corner<3>::null());

  CHECK(Corner<3>::tnw() != Corner<3>::bsw());
  CHECK(Corner<3>::tnw() != Corner<3>::bse());
  CHECK(Corner<3>::tnw() != Corner<3>::bnw());
  CHECK(Corner<3>::tnw() != Corner<3>::bne());
  CHECK(Corner<3>::tnw() != Corner<3>::tsw());
  CHECK(Corner<3>::tnw() != Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tnw() != Corner<3>::tnw());
  CHECK(Corner<3>::tnw() != Corner<3>::tne());
  CHECK(Corner<3>::tnw() != Corner<3>::null());

  CHECK(Corner<3>::tne() != Corner<3>::bsw());
  CHECK(Corner<3>::tne() != Corner<3>::bse());
  CHECK(Corner<3>::tne() != Corner<3>::bnw());
  CHECK(Corner<3>::tne() != Corner<3>::bne());
  CHECK(Corner<3>::tne() != Corner<3>::tsw());
  CHECK(Corner<3>::tne() != Corner<3>::tse());
  CHECK(Corner<3>::tne() != Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::tne() != Corner<3>::tne());
  CHECK(Corner<3>::tne() != Corner<3>::null());

  CHECK(Corner<3>::null() != Corner<3>::bsw());
  CHECK(Corner<3>::null() != Corner<3>::bse());
  CHECK(Corner<3>::null() != Corner<3>::bnw());
  CHECK(Corner<3>::null() != Corner<3>::bne());
  CHECK(Corner<3>::null() != Corner<3>::tsw());
  CHECK(Corner<3>::null() != Corner<3>::tse());
  CHECK(Corner<3>::null() != Corner<3>::tnw());
  CHECK(Corner<3>::null() != Corner<3>::tne());
  CHECK_FALSE(Corner<3>::null() != Corner<3>::null());
}
TEST_CASE("Corner<2> <")
{
  CHECK_FALSE(Corner<2>::sw() < Corner<2>::sw());
  CHECK(Corner<2>::sw() < Corner<2>::se());
  CHECK(Corner<2>::sw() < Corner<2>::nw());
  CHECK(Corner<2>::sw() < Corner<2>::ne());
  CHECK(Corner<2>::sw() < Corner<2>::null());

  CHECK_FALSE(Corner<2>::se() < Corner<2>::sw());
  CHECK_FALSE(Corner<2>::se() < Corner<2>::se());
  CHECK(Corner<2>::se() < Corner<2>::nw());
  CHECK(Corner<2>::se() < Corner<2>::ne());
  CHECK(Corner<2>::se() < Corner<2>::null());

  CHECK_FALSE(Corner<2>::nw() < Corner<2>::sw());
  CHECK_FALSE(Corner<2>::nw() < Corner<2>::se());
  CHECK_FALSE(Corner<2>::nw() < Corner<2>::nw());
  CHECK(Corner<2>::nw() < Corner<2>::ne());
  CHECK(Corner<2>::nw() < Corner<2>::null());

  CHECK_FALSE(Corner<2>::ne() < Corner<2>::sw());
  CHECK_FALSE(Corner<2>::ne() < Corner<2>::se());
  CHECK_FALSE(Corner<2>::ne() < Corner<2>::nw());
  CHECK_FALSE(Corner<2>::ne() < Corner<2>::ne());
  CHECK(Corner<2>::ne() < Corner<2>::null());

  CHECK_FALSE(Corner<2>::null() < Corner<2>::sw());
  CHECK_FALSE(Corner<2>::null() < Corner<2>::se());
  CHECK_FALSE(Corner<2>::null() < Corner<2>::nw());
  CHECK_FALSE(Corner<2>::null() < Corner<2>::ne());
  CHECK_FALSE(Corner<2>::null() < Corner<2>::null());
}
TEST_CASE("Corner<3> <")
{
  CHECK_FALSE(Corner<3>::bsw() < Corner<3>::bsw());
  CHECK(Corner<3>::bsw() < Corner<3>::bse());
  CHECK(Corner<3>::bsw() < Corner<3>::bnw());
  CHECK(Corner<3>::bsw() < Corner<3>::bne());
  CHECK(Corner<3>::bsw() < Corner<3>::tsw());
  CHECK(Corner<3>::bsw() < Corner<3>::tse());
  CHECK(Corner<3>::bsw() < Corner<3>::tnw());
  CHECK(Corner<3>::bsw() < Corner<3>::tne());
  CHECK(Corner<3>::bsw() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::bse() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bse() < Corner<3>::bse());
  CHECK(Corner<3>::bse() < Corner<3>::bnw());
  CHECK(Corner<3>::bse() < Corner<3>::bne());
  CHECK(Corner<3>::bse() < Corner<3>::tsw());
  CHECK(Corner<3>::bse() < Corner<3>::tse());
  CHECK(Corner<3>::bse() < Corner<3>::tnw());
  CHECK(Corner<3>::bse() < Corner<3>::tne());
  CHECK(Corner<3>::bse() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::bnw() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bnw() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bnw() < Corner<3>::bnw());
  CHECK(Corner<3>::bnw() < Corner<3>::bne());
  CHECK(Corner<3>::bnw() < Corner<3>::tsw());
  CHECK(Corner<3>::bnw() < Corner<3>::tse());
  CHECK(Corner<3>::bnw() < Corner<3>::tnw());
  CHECK(Corner<3>::bnw() < Corner<3>::tne());
  CHECK(Corner<3>::bnw() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::bne() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::bne() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::bne() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::bne() < Corner<3>::bne());
  CHECK(Corner<3>::bne() < Corner<3>::tsw());
  CHECK(Corner<3>::bne() < Corner<3>::tse());
  CHECK(Corner<3>::bne() < Corner<3>::tnw());
  CHECK(Corner<3>::bne() < Corner<3>::tne());
  CHECK(Corner<3>::bne() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::tsw() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tsw() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tsw() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tsw() < Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tsw() < Corner<3>::tsw());
  CHECK(Corner<3>::tsw() < Corner<3>::tse());
  CHECK(Corner<3>::tsw() < Corner<3>::tnw());
  CHECK(Corner<3>::tsw() < Corner<3>::tne());
  CHECK(Corner<3>::tsw() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::tse() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tse() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tse() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tse() < Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tse() < Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tse() < Corner<3>::tse());
  CHECK(Corner<3>::tse() < Corner<3>::tnw());
  CHECK(Corner<3>::tse() < Corner<3>::tne());
  CHECK(Corner<3>::tse() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tnw() < Corner<3>::tnw());
  CHECK(Corner<3>::tnw() < Corner<3>::tne());
  CHECK(Corner<3>::tnw() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::tne() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::bne());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::tse());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::tne() < Corner<3>::tne());
  CHECK(Corner<3>::tne() < Corner<3>::null());

  CHECK_FALSE(Corner<3>::null() < Corner<3>::bsw());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::bse());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::bnw());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::bne());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::tsw());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::tse());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::tnw());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::tne());
  CHECK_FALSE(Corner<3>::null() < Corner<3>::null());
}
TEST_CASE("Test ostream for Corner<2>")
{
  stringstream ss;
  ss << Corner<2>::sw();
  CHECK(ss.str() == "Corner<2>::sw()");
  ss.str("");
  ss << Corner<2>::se();
  CHECK(ss.str() == "Corner<2>::se()");
  ss.str("");
  ss << Corner<2>::nw();
  CHECK(ss.str() == "Corner<2>::nw()");
  ss.str("");
  ss << Corner<2>::ne();
  CHECK(ss.str() == "Corner<2>::ne()");
  ss.str("");
  ss << Corner<2>::null();
  CHECK(ss.str() == "Corner<2>::null()");
  ss.str("");
  ss << Corner<2>(13);
  CHECK(ss.str() == "Corner<2> invalid value: 13");
}
TEST_CASE("Test ostream for Corner<3>")
{
  stringstream ss;
  ss << Corner<3>::bsw();
  CHECK(ss.str() == "Corner<3>::bsw()");
  ss.str("");
  ss << Corner<3>::bse();
  CHECK(ss.str() == "Corner<3>::bse()");
  ss.str("");
  ss << Corner<3>::bnw();
  CHECK(ss.str() == "Corner<3>::bnw()");
  ss.str("");
  ss << Corner<3>::bne();
  CHECK(ss.str() == "Corner<3>::bne()");
  ss.str("");
  ss << Corner<3>::tsw();
  CHECK(ss.str() == "Corner<3>::tsw()");
  ss.str("");
  ss << Corner<3>::tse();
  CHECK(ss.str() == "Corner<3>::tse()");
  ss.str("");
  ss << Corner<3>::tnw();
  CHECK(ss.str() == "Corner<3>::tnw()");
  ss.str("");
  ss << Corner<3>::tne();
  CHECK(ss.str() == "Corner<3>::tne()");
  ss.str("");
  ss << Corner<3>::null();
  CHECK(ss.str() == "Corner<3>::null()");
  ss.str("");
  ss << Corner<3>(13);
  CHECK(ss.str() == "Corner<3> invalid value: 13");
}
TEST_CASE("Test iterator for Corner<2>")
{
  auto iter = Corner<2>::getValues().begin();
  CHECK(iter == Corner<2>::getValues().begin());
  CHECK(iter != Corner<2>::getValues().end());
  CHECK(*iter == Corner<2>::sw());
  ++iter;
  CHECK(iter->getIndex() == 1);
  CHECK(*iter == Corner<2>::se());
  ++iter;
  CHECK(*iter == Corner<2>::nw());
  ++iter;
  CHECK(*iter == Corner<2>::ne());
  ++iter;
  CHECK(*iter == Corner<2>::null());
  CHECK(iter == Corner<2>::getValues().end());
}
TEST_CASE("Test iterator for Corner<3>")
{
  auto iter = Corner<3>::getValues().begin();
  CHECK(iter == Corner<3>::getValues().begin());
  CHECK(iter != Corner<3>::getValues().end());
  CHECK(*iter == Corner<3>::bsw());
  ++iter;
  CHECK(iter->getIndex() == 1);
  CHECK(*iter == Corner<3>::bse());
  ++iter;
  CHECK(*iter == Corner<3>::bnw());
  ++iter;
  CHECK(*iter == Corner<3>::bne());
  ++iter;
  CHECK(*iter == Corner<3>::tsw());
  ++iter;
  CHECK(*iter == Corner<3>::tse());
  ++iter;
  CHECK(*iter == Corner<3>::tnw());
  ++iter;
  CHECK(*iter == Corner<3>::tne());
  ++iter;
  CHECK(*iter == Corner<3>::null());
  CHECK(iter == Corner<3>::getValues().end());
}
