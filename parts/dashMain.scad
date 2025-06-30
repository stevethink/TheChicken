$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

thickness = 5.3; // Thickness of the plywood
centerAngle = 22;
outerRadius = 190;
frontEdgeLength = 940;

module sideShape() {
  cube([frontEdgeLength, 10, thickness]);
  translate([frontEdgeLength, outerRadius, 0])
    cylinder(h = thickness, r = outerRadius);
}


module top() {
//  difference()
  {
    hull() intersection() {
      union() { 
        sideShape();
        mirror([1, 0, 0])
          rotate([0, 0, centerAngle])
            sideShape();
        rotate([0, 0, -centerAngle / 2])
          translate([-2159 / 2, 402, 0])
            cube([2159, 10, thickness]);
      }

      rotate([0, 0, -centerAngle / 2])
        translate([-1100, 0, 0])
          cube([2200, 270, thickness]);
    }
/*    hull()
    rotate([0, 0, -centerAngle / 2])
      for (i = [0, 1])
        translate([1100 - (i * 711), 370, -1]) {
          translate([-178 - 25, 0, 0])
            cube([50, 50, thickness + 2]);
          translate([-178, -75, 0])
            cylinder(h = thickness + 2, d = 50);
        }
    */
  }
}

module topLeft() {
  intersection() {
    top();
    rotate([0, 0, -centerAngle / 2])
      cube([1200, 370, thickness]);
  }
}

module topRight() {
  intersection() {
    top();
    rotate([0, 0, -centerAngle / 2])
      mirror([1, 0, 0])
        cube([1200, 370, thickness]);
  }
}

//rotate([0,0,90 + centerAngle / 2])
//projection()
top();
//topRight();
//topLeft();
