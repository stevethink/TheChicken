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


//projection()
difference() {
  hull()
  intersection() {
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
        cube([2200, 407, thickness]);
  }
  rotate([0, 0, -centerAngle / 2])
    translate([714, 407 + 15, -0.1])
      cylinder(h = thickness + 0.2, d = 90);
}
