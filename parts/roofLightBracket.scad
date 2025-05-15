//$fn = 500;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

bracket_thickness = 6; // Thickness of the mount
// bracket_thickness = 0.6; // Thickness of the mount
scale_coef = 1.088;

module bracket() {
  difference() {
    for (i = [0, 1]) {
      scale([i == 0 ? 1 : scale_coef, i == 0 ? 1 : scale_coef, i == 0 ? 1 : scale_coef]) hull() {
        translate([0, 0, i == 0 ? -1.5 : 0]) {
          cylinder(h = bracket_thickness, d = 54.6);  
          translate([0, 94, 0])
            cylinder(h = bracket_thickness, d = 21);
          for (i = [0, 1]) mirror([i, 0, 0])
            intersection() {
              translate([-317.6, -12, 0])
                cylinder(h = bracket_thickness, d = 690);
              translate([0, 0, -1])
                cube([50, 100, 16]);
            }
          }
      }
    }

// for rear side
/*    translate([0, 42, 1201.5])
      rotate([0, 90, 0])
        cylinder(h = 100, d = 2400, center = true);
*/

// for rear center
    translate([0, 42, 651.5])
      rotate([0, 90, 0])
        cylinder(h = 100, d = 1300, center = true);

    
// for front
/*    translate([0, 42, 751.5])
      rotate([0, 90, 0])
        cylinder(h = 100, d = 1500, center = true);
*/

    translate([0, -5, 0]) {
      translate([0, 44,0])
        cube([20, 36, 16], center = true);
      
      cylinder(h = 16, d = 16.8, center = true);
      translate([0, 77, 0])
        cylinder(h = 16, d = 16.8, center = true);
    }
  }
  for (i = [0, 1]) mirror([i, 0, 0])
// for front left center
//    translate([19.4, 12 + (i == 1 ? -1 : 0), 0])
    translate([19.4, 12, 0])
      cylinder(h = 4.2, d = 7.6);
}

bracket();

// translate([0, 115, -1.5])
//  cube([1, 1, 7.2]);
