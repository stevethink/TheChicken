// $fn = 256;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

// Parameters for both mounts
mount_thickness = 2.6; // Thickness of the mount
//mount_thickness = 0.5; // Thickness of the mount
mount_size = [111, 122, mount_thickness];

explode = true;

module mount() {
  difference() {
    cube(mount_size);

    translate([mount_size[0] / 2, mount_size[1] / 2, -0.1])
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([mount_size[0] / 2 - 4, mount_size[1] / 2 - 4, 0])
                cylinder(h = 3, d = 3);

    translate([mount_size[0] - 20, 17, -0.1])
      cylinder(h = 3, d = 3);
  }

  translate([mount_size[0] - 20 - 1.5, 17 + 3, 0]) {
    cube([3, 6, mount_thickness + 2]);
    translate([0, -12, 0])
      cube([3, 6, mount_thickness + 2]);
  }

  translate([43.5, 34, 0])
    for (i = [0, 1])
      for (j = [0, 1])
        mirror([i, 0, 0])
          mirror([0, j, 0])
            translate([32.95, 22.5, 0])
              difference() {
                cylinder(h = 7, d = 6);
                cylinder(h = 10, d = 3);
              }
                  
  translate([55.5, 90, 0])
    for (i = [0, 1])
      for (j = [0, 1])
        mirror([i, 0, 0])
          mirror([0, j, 0])
            translate([46.5, 19.7, 0])
              difference() {
                cylinder(h = 9, d = 5);
                cylinder(h = 10, d = 2);
              }
}

module cover() {
  difference() {
    union() {
      difference() {
        cube(mount_size + [0, 0, 30]);
        translate([mount_thickness, mount_thickness, -0.1])
          cube(mount_size + [-2 * mount_thickness, -2 * mount_thickness, 30 - mount_thickness]);
      }
      
      intersection() {
        cube(mount_size + [0, 0, 30]);
        translate([mount_size[0] / 2, mount_size[1] / 2, 0])
          for (i = [0, 1])
            for (j = [0, 1])
              mirror([i, 0, 0])
                mirror([0, j, 0])
                  translate([mount_size[0] / 2, mount_size[1] / 2, 0])
                    cylinder(h = mount_thickness + 30, d = 24);
      }
    }

    translate([mount_size[0] / 2, mount_size[1] / 2, -0.1])
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([mount_size[0] / 2, mount_size[1] / 2, mount_thickness]) {
                cylinder(h = mount_thickness + 30, d = 19);
                translate([-4, -4, -mount_thickness - 0.1])
                  cylinder(h = mount_thickness + 0.2, d = 3.6);
              }

    translate([-0.1, -0.1, mount_thickness - 0.1])
      cube([mount_size[0] + 0.2, 20, 35]);
              
    translate([mount_size[0] / 2 - (22 / 2), mount_size[1] - 17, 2 * mount_thickness])
      cube([36, 20, 35]);
              
    translate([-0.1, mount_size[1] - 46, mount_thickness])
      cube([10, 22, 18]);
              
  }    
}

//mirror([0, 0, 1])
//  mount();



translate([0, 0, 10]) {
  cover();
}
