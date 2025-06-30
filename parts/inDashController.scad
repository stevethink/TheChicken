// $fn = 256;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

// Parameters for both mounts
thickness = 5.3; // Thickness of the plywood
mount_thickness = 2.6; // Thickness of the mount
//mount_thickness = 0.5; // Thickness of the mount
pcb_size = [90, 97, 1.6];
mount_size = [138, pcb_size[1], mount_thickness];
di_size = [95, 71, 1.6];
di_mount_size = [102, 71, mount_thickness];
do_mount_size = [95, 51.7, mount_thickness];
frame_size = [110, 192, 0];
frame_height = 11 + 2 * mount_thickness;

explode = true;

module gps_mount() {
  difference() {
    union() {
      cube([20, 15.2 + 2 * mount_thickness, mount_thickness]);
      translate([20 - mount_thickness, 0, 0])
        cube([mount_thickness, 15.2 + 2 * mount_thickness, 15.3 + 2 * mount_thickness]);
    }
    translate([4, 4, -0.1])
      cylinder(mount_thickness + 0.2, d = 3.2);
    translate([20 - mount_thickness - 0.1, mount_thickness, mount_thickness])
      cube([mount_thickness + 0.2, 15.2, 15.3]);
  }
}

module pcb_bosses(diameter, height) {
  for (i = [0, 1])
    for (j = [0, 1])
      mirror([i, 0, 0])
        mirror([0, j, 0])
          translate([41.3, 46.4, 0])
            cylinder(h = height, d = diameter);
}

module mount() {
  rotate([0, 0, -90])
    translate([-mount_size[0] / 2, -mount_size[1] / 2, 0])
  difference() {
    union() {
      cube(mount_size);
  //    translate([pcb_size[0] / 2 + 21, pcb_size[1] / 2, 0])
    //    pcb_bosses(5, 11.6);
    }
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
    }
}

module di_mount() {
  difference() {
    union() {
      translate([(pcb_size[0] - di_size[0])/ 2 + 14, mount_size[1] - di_size[1] + 2, 0])
        cube(di_mount_size);
      translate([mount_size[0] / 2 - 3, 0, 0]) {
        translate([33, 0, 0])
          cube([14.5, 30, di_mount_size[2]]);
        translate([-54.5, 0, 0])
          cube([17.5, 30, di_mount_size[2]]);
      }
      translate([pcb_size[0] / 2 + 21, pcb_size[1] / 2, 0])
        pcb_bosses(5, 16);
    }
    translate([pcb_size[0]/ 2 + 14, mount_size[1] - di_size[1] / 2 + 2, -0.1])
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([43.5, 32, 0])
                cylinder(h = di_mount_size[2] + 0.2, d = 3);
    translate([pcb_size[0] / 2 + 21, pcb_size[1] / 2, -0.1])
      pcb_bosses(1.8, 19);
  }
}

module do_mount() {
  difference() {
    union() {
      cube(do_mount_size);
      translate([do_mount_size[0], do_mount_size[1] / 2 - 8, 0])
        difference() {
          cube([20, 15.2 + 2 * mount_thickness, mount_thickness]);
          translate([4, 4, -0.1])
            cylinder(mount_thickness + 0.2, d = 3);
        }

      translate([do_mount_size[0] / 2, do_mount_size[1] / 2, 0]) {
        for (i = [0, 1])
          for (j = [0, 1])
            mirror([i, 0, 0])
              mirror([0, j, 0])
                translate([do_mount_size[0] / 2 - 6.5, do_mount_size[1] / 2 - mount_thickness, 0]) {
                  cube([6.5, mount_thickness, 11]);
                  translate([0, 0, 11])
                    cube([6.5, 13, mount_thickness]);
                }
        }
    }
    translate([do_mount_size[0] - 15, do_mount_size[1] + 5, -0.1])
      cylinder(mount_thickness + 0.2, d = 19.7);
    translate([do_mount_size[0]/ 2, do_mount_size[1] / 2, -0.1]) {
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([43.5, 32, 11])
                cylinder(h = di_mount_size[2] + 0.2, d = 3.4);
      translate([-11, 0, 0])
        for (i = [0, 1])
          for (j = [0, 1])
            mirror([i, 0, 0])
              mirror([0, j, 0])
                translate([32.95, 22.5, 0])
                  cylinder(h = 20, d = 3);
    }
  }
}

module frame_shape(corner_radius, taper, size) {
  hull()
    for (i = [0, 1])
      for (j = [0, 1])
        mirror([i, 0, 0])
          mirror([0, j, 0])
            translate([ size[0] / 2 - corner_radius, size[1] / 2 - corner_radius, 0])
              cylinder(size[2], corner_radius, corner_radius - taper);
}

module frame_mount() {
  difference() {
    union() {
      frame_shape(6.5, 0, frame_size + [2 * mount_thickness, 2 * mount_thickness, mount_thickness]);
      frame_shape(13, 0, frame_size + [2 * mount_thickness, -10, frame_height]);
    }
    translate([1, -1.3, -0.1]) {
      frame_shape(0.2, 0, frame_size + [-9, -25, frame_height - 2 * mount_thickness - 3.1]);
      translate([0, 2, 0])
        frame_shape(0.2, 0, frame_size + [-9, -78, 30]);
    }
    translate([frame_size[0] / 2 - 8, 0, 0])
      cube([13, 45, 40], center = true);   

    translate([-frame_size[0] / 2, -19, frame_height / 2 + 2 * mount_thickness])
      cube([15, 75, frame_height], center = true);   

    for (i = [0, 1])
      mirror([0, i, 0])
        translate([0, frame_size[1] / 2, frame_height / 2 + 2 * mount_thickness])
          cube([42, frame_size[1] / 2, frame_height], center = true);   

    translate([-1.5, -0.8, 0])
      for (i = [0, 1])
        for (j = [0, 1])
          mirror([i, 0, 0])
            mirror([0, j, 0])
              translate([32.8, 63.1, 0])
                cylinder(h = 20, d = 4);

    for (i = [0, 1])
      for (j = [0, 1])
        mirror([i, 0, 0])
          mirror([0, j, 0])
            translate([j == 1 ? 42.5 : 26, 86, frame_height - 1.5])
              cylinder(h = 20, d = 6.4);
  }
}

module frame_mount_inside() {
  translate([1, -1.3, -0.1])
    frame_shape(0.2, 0, frame_size + [-9, -25, frame_height - 2 * mount_thickness - 3.1]);
  translate([frame_size[0] / 2 - 8, 0, 0])
    cube([13, 45, 40], center = true);   
}

module frame_template() {
  difference() {
    cube(frame_size + [20, 10, 5], center = true);
    translate([0, 0, -10])
      frame_shape(13, 0, frame_size + [2 * mount_thickness + 0.9, -9.3, 35]);
  }
}

module test_stand() {
  difference() {
    cube(frame_size + [20, 10, 5], center = true);
    translate([0, 0, -10])
      frame_shape(13, 0, frame_size + [2 * mount_thickness + 1.9, -8, 35]);
  }
  
  for (i = [0, 1])
    mirror([0, i, 0])
      translate([0, frame_size[1] / 2 + 0.5, 70])
        cube([frame_size[0], 8, 140], center = true);
}


module cam_screen_mount() {
  difference() {
    translate([-17, -15, 0])
      cube([45, 45, 45]);

    translate([0, 0, -0.1]) {
      cylinder(6, d = 18.4);
      cylinder(13, d = 12.3);
      cylinder(46, d = 5);
    }
    
    translate([0, 0, 22])
      cylinder(50, d = 12.3);
    
    translate([5.5, 48, 22.5])
      for (i = [0, 1]) for (j = [0, 1])
        mirror([i, 0, 0])
          mirror([0, 0, j])
            translate([18, 0, 18])
              rotate([90, 0, 0])
                cylinder(70, d = 5.2);
    /*

    translate([0, 22.7, 35])
      rotate([90, 0, 0]) {
        cylinder(60, d = 4.5);
        translate([0, 0, mount_thickness])
          cylinder(20, d = 7.7);
      }
    */
  }
  
  for (i = [0, 1])
    mirror([i, 0, 0])
      translate([4.5, -2, 6])
        cube([2, 4, 12 + mount_thickness]);
}

module headlight_switch_cover() {
  across_flats = 17.1;  // Distance between two opposing flat sides
  thickness = 3;      // Nut thickness
  height = 6;

  // Calculate the circumscribed circle radius of the hexagon
  radius = across_flats / (2 * cos(30)); // cos(30°) = √3 / 2

  difference() {
    union() {
      cylinder(h = height, d = 27);
      translate([0, 0, thickness])
        cylinder(h = height - thickness, d = 32);
    }

    // Outer hexagon shape
    cylinder(h=thickness, r=radius, $fn=6);
    
    cylinder(h = height, d = 5.3);
  }
}

module console() {
//  rotate([0, 0, -centerAngle / 2])
//    translate([216, 270, 0])
  
  difference() 
  {
    cube([834.5, 140, thickness]);
    translate([465, 69, -1])
      rotate([0, 0, 90])
        frame_mount_inside();
        // frame_shape(13, 0, frame_size + [2 * mount_thickness + 0.6, -9.4, frame_height]);

    translate([685, 54, -1])
      rotate([90, 0, 0])
        translate([5.5, 48, 22.5])
          for (i = [0, 1]) for (j = [0, 1])
            mirror([i, 0, 0])
              mirror([0, 0, j])
                translate([18, 0, 18])
                  rotate([90, 0, 0])
                    cylinder(70, d = 5.2);

  hull() for (i = [0, 1])
    translate([100, 75 + i * 43.5, -1])
      cylinder(10, d = 8);

  }  
}

// frame_mount_inside();

projection()
console();

// headlight_switch_cover();

// cam_screen_mount();

// test_stand();

//translate([465, 69, -10])
//  rotate([0, 0, 90])
  //  frame_mount();

//translate([0, 0, explode ? 25 : 0])
//  mount();

/*
translate([0, 3, explode ? 40 : 0])
  rotate([0, 0, 90])
    cube(pcb_size, center = true);
*/

//translate([-di_mount_size[0] / 2, -di_mount_size[1] / 2 - 27, explode ? 60 : 0])
//  rotate([180, 0, 90])
//    di_mount();

/*
translate([do_mount_size[0] / 2 - 10, do_mount_size[1] / 2 + 18, explode ? 77 : 0])
  rotate([180, 0, -90])
    do_mount();

translate([20, -53, explode ? 72 : 0])
  rotate([180, 0, -90])
    gps_mount();
*/

//translate([0, 55, 0])
//cube([2, 2, 7.8]);
/*
rotate([0, 0, -90])
  translate([-mount_size[0] / 2, -mount_size[1] / 2, 20])
    mount();
*/

// frame_template();
/*
difference() {
  translate([0, 0, -20])
    frame_mount();
  translate([0, 0, -20])
    cube(frame_size + [20, 10, 10], center = true);
}
*/


// spacers
//difference() {
//  cylinder(h = 4, d = 8);
//  cylinder(h = 4, d = 3.4);
//}
