// $fn = 256;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

mount_thickness = 2.6; // Thickness of the mount

module cam_screen_mount() {
  difference() {
    union() {
      hull() {
        cylinder(12 + mount_thickness, d = 12.3 + 2 * mount_thickness);
        for (i = [0, 1])
          mirror([i, 0, 0])
            translate([30, 20, 0])
              cube([mount_thickness, mount_thickness, 12 + mount_thickness]);
      }
      
      hull() {
        translate([-(12 + mount_thickness) / 2, 0, 10]) {
          translate([0, 12.3 / 2 - mount_thickness, 0]) 
            cube([12 + mount_thickness, mount_thickness, mount_thickness]);              
    
          translate([0, 20, 0]) {
            cube([12 + mount_thickness, mount_thickness, mount_thickness]);
            translate([0, 0, 30]) 
              cube([12 + mount_thickness, mount_thickness, mount_thickness]);              
          }
        }
      }
    }
    translate([0, 0, -0.1]) {
      cylinder(12, d = 12.3);
      cylinder(20, d = 5);
    }
    for (i = [0, 1])
      mirror([i, 0, 0])
        translate([25, 22.7, (12 + mount_thickness) / 2])
          rotate([90, 0, 0]) {
            cylinder(60, d = 4.5);
            translate([0, 0, mount_thickness])
              cylinder(20, d = 7.7);
          }
    translate([0, 22.7, 35])
      rotate([90, 0, 0]) {
        cylinder(60, d = 4.5);
        translate([0, 0, mount_thickness])
          cylinder(20, d = 7.7);
      }
  }
  
  for (i = [0, 1])
    mirror([i, 0, 0])
      translate([4.5, -2, 0])
        cube([2, 4, 12 + mount_thickness]);
}

cam_screen_mount();
