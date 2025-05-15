// $fn = 256;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

// Parameters for both mounts
mount_thickness = 2.6; // Thickness of the mount
// mount_thickness = 0.5; // Thickness of the mount

module mount() {
  difference() {
    union() {
  //    cube([90, 90, mount_thickness], center = true);
      cube([94, 170, mount_thickness], center = true);
      intersection() {
        union() {
          for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
            translate([40.25, 40.25, 0]) {
              cylinder(h = 20, d = 11);
              cylinder(h = 15, d = 17);
            }
          for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
            translate([-40.25, 40.25, 0])
              cylinder(h = 17, d = 17);
        }
        cube([94, 139, 55], center = true);        
      }
      
//      translate([pcb_size[0] / 2 + 21, pcb_size[1] / 2, 0])
  //      pcb_bosses(5, 11.6);
    }
    for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
    {
//      translate([35, 61.625, 0])
      translate([34, 77, 0])
        cylinder(h = 5, d = 3.9, center = true);
      translate([40.25, 40.25, 0]) {
        cylinder(h = 55, d = 5.2, center = true);
        cylinder(h = 8, d = 11, center = true);
      }
    }
    /*
    translate([mount_size[0] / 2, mount_size[1] / 2 - 2.5, -0.1])
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([63, 32.5, 0])
                cylinder(h = 5, d = 3.4);
    translate([pcb_size[0] / 2 + 21, pcb_size[1] / 2, -0.2])
      pcb_bosses(2.2, 12);

    translate([mount_size[0] / 2 - 3, mount_size[1] / 2, -0.1])
      for (i = [0, 1])
        mirror([i, 0, 0])
          translate([pcb_size[0] / 2, -18, 0])
            cube([30, 40, 5]);
    */
    }
}

// mount();

module mount_2() {
  difference() {
    union() {
      cube([221, 94, mount_thickness], center = true);
      intersection() {
        union() {
          translate([0, 0, 0]) {
            for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
              translate([40.25, 40.25, 0]) {
                cylinder(h = 20, d = 11);
                cylinder(h = 15, d = 17);
              }
            for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
              translate([-40.25, 40.25, 0])
                cylinder(h = 17, d = 17);
          }
          for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
            translate([94, 40, 0])
              cylinder(h = 27.6, d = 10);
        }
        cube([350, 94, 55], center = true);        
      }
    }
    translate([26, 0, 0])
      for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
        translate([77.1, 34.1, 0])
          cylinder(h = 5, d = 3.9, center = true);
    translate([-85, 0, 0])
      for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
        translate([-19, 39.5, 0])
          cylinder(h = 5, d = 3.9, center = true);
    translate([0, 0, 0])
      for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
        translate([40.25, 40.25, 0]) {
          cylinder(h = 55, d = 5.2, center = true);
          cylinder(h = 8, d = 11, center = true);
        }
  }
}

intersection() {
 // mount_2();
 // cube([221, 94, 0.2], center = true);
}

module mount_3() {
  difference() {
    union() {
      // cube([100, 100, mount_thickness], center = true);
      difference() {
        hull() for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
          translate([40.25, 40.25, 0])
            cylinder(h = 32.2, d = 24);
        hull() for (i = [0, 1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
          translate([40.25, 40.25, mount_thickness])
            cylinder(h = 40, d = 20);
        translate([-10, 0, mount_thickness + 15])
          cube([100, 60, 30], center = true);
      }
      for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
        translate([40.25, 40.25, 0]) {
          cylinder(h = 23, d = 11);
          cylinder(h = 18, d = 20);
        }
      for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
        translate([-40.25, 40.25, 0])
          cylinder(h = 20, d = 20);
    }
    translate([0, 0, 0])
      for (i = [0, 1]) mirror([i, 0, 0]) mirror([0, i, 0])
        translate([40.25, 40.25, 0]) {
          cylinder(h = 55, d = 5.2, center = true);
          // cylinder(h = 8, d = 11, center = true);
        }
  }
}

intersection() {
  mount_3();
 // cube([221, 94, 0.2], center = true);
}
