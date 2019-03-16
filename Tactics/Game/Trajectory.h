#pragma once

namespace Game
{

enum class Trajectory {
    // Vector
	None,		 // target self or instant, can't be avoided
    Straight,    // forward relative to direction
    Reverse,     // backwards relative to direction
    Forehand,    // right to left, relative to direction
    Backhand,    // left to right, relative to direction
    Down,        // up to down
    Up,          // down to up
};

}    // ::Game
