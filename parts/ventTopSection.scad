$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

thickness = 2.6; // Thickness of the mount
length = 170;
height = 35;
gap_width = 45;
lip_width = 20;

air_scoop_radius = 15;
air_scoop_length = 80;

air_slot_diameter = 7;
air_slot_length = 70;

length_left = length + 35;
length_right = length + 5;

module airScoopHull(inside) {
  sphere(air_scoop_radius - (inside ? thickness : 0));
  cylinder(h = air_scoop_length, r = air_scoop_radius - (inside ? thickness : 0));
  if (inside) {
    translate([0, 0, air_scoop_length])
      sphere(air_scoop_radius - thickness);
  }
}

module airScoop() {
  difference() {
    airScoopHull(false);
    airScoopHull(true);
    translate([0, -30, -30])
      cube([60, 60, air_scoop_length + 60]);
  }
}

module airSlot() {
  cube([air_slot_length, air_slot_diameter, 3 * thickness + 1], center = true);
  for (i = [0, 1]) mirror([i, 0, 0]) {
    translate([air_slot_length / 2, 0, 0])
      cylinder(h = 3 * thickness + 1, d = air_slot_diameter, center = true);
  }
}

module airSlotShort() {
  cube([42, air_slot_diameter, 3 * thickness + 1], center = true);
  for (i = [0, 1]) mirror([i, 0, 0]) {
    translate([42 / 2, 0, 0])
      cylinder(h = 3 * thickness + 1, d = air_slot_diameter, center = true);
  }
}

module ventSection() {
  difference() {
    union() {
      cube([length, lip_width, thickness]);
      translate([0, lip_width, 0])
        rotate([atan2(height, gap_width), 0, 0])
          cube([length, sqrt((gap_width^2) + (height^2)) + 0.891, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([length, lip_width + left_fill, thickness]);
      
/*
      translate([(length - air_scoop_length) / 2, lip_width + (gap_width / 2), height / 2])
        rotate([0, 90, 0]) rotate([0, 0, atan2(height, gap_width)])
          airScoop();
      */
    }
    for (j = [0, 1]) {
      for (i = [0 : 2]) 
        translate([lip_width / 2 + (i * (length - lip_width) / 2), lip_width / 2 + (j * (gap_width + lip_width)), 0])
          cylinder(h = 100, d = 3.5);

      translate([(j * 2 + 1) * length / 4, lip_width + (gap_width / 2), height / 2 + (thickness / 2)])
        rotate([0, 0, 0]) rotate([atan2(height, gap_width), 0, 0])
          airSlot();      
    }
  }
}

module ventSectionWithWiperHole() {
  difference() {
    union() {
      cube([length, lip_width, thickness]);
      translate([0, lip_width, 0])
        rotate([atan2(height, gap_width), 0, 0])
          cube([length, sqrt((gap_width^2) + (height^2)) + 0.891, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([length, lip_width + left_fill, thickness]);
      
    }
    for (j = [0, 1]) {
      for (i = [0 : 2]) 
        translate([lip_width / 2 + (i * (length - lip_width) / 2) - (i == 1 ? (j * 10) : 0), lip_width / 2 + (j * (gap_width + lip_width)), 0])
          cylinder(h = 100, d = 3.5);

      translate([(j * 2 + 1) * length / 4, lip_width + (gap_width / 2), height / 2 + (thickness / 2)])
        rotate([0, 0, 0]) rotate([atan2(height, gap_width), 0, 0])
          airSlot();      
    }

    translate([95, 84, 40])
      cylinder(h = 20, d = 22, center = true);
  }
}

module centerVentSectionRight() {
  difference() {
    union() {
      cube([length_right, lip_width, thickness]);
      translate([0, lip_width, 0])
        rotate([atan2(height, gap_width), 0, 0])
          cube([length_right, sqrt((gap_width^2) + (height^2)) + 0.891, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([length_right, lip_width + left_fill, thickness]);
   }
    translate([length_right, 0, -1])
      rotate([0, 0, 10])
          cube([50, 100, 50]);
    translate([length_right - 38, 0, 0])
      rotate([0, 0, 10])
        translate([0, -10, -1])
          cube([50, 25, 2 * thickness]);

    for (i = [0 : 2]) {
      translate([lip_width / 2 + (i * (length_right - lip_width - 38) / 2), lip_width / 2, 0])
        cylinder(h = 100, d = 3.5);
      if (i != 1)
      translate([lip_width / 2 + (i * (length_right - lip_width - 14) / 2), lip_width / 2 + gap_width + lip_width, 0])
        cylinder(h = 100, d = 3.5);
    }
  }
}

module centerVentSectionLeft() {
  mirror([1, 0, 0]) difference() {
    union() {
      cube([length_left, lip_width, thickness]);
      translate([0, lip_width, 0])
        rotate([atan2(height, gap_width), 0, 0])
          cube([length_left, sqrt((gap_width^2) + (height^2)) + 0.891, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([length_left, lip_width + left_fill, thickness]);
    }
    translate([length_left, 0, -1])
      rotate([0, 0, 10])
          cube([50, 100, 50]);
    translate([length_left - 38, 0, 0])
      rotate([0, 0, 10])
        translate([0, -10, -1])
          cube([50, 25, 2 * thickness]);

    for (i = [0 : 2]) {
      translate([lip_width / 2 + (i * (length_left - lip_width - 38) / 2), lip_width / 2, 0])
        cylinder(h = 100, d = 3.5);
      if (i != 1)
      translate([lip_width / 2 + (i * (length_left - lip_width - 14) / 2), lip_width / 2 + gap_width + lip_width, 0])
        cylinder(h = 100, d = 3.5);
    }
  }
}

/*
module centerVentSectionRight() {
  difference() {
    union() {
      cube([length_right, lip_width, thickness]);
      translate([0, lip_width, 0])
        rotate([atan2(height, gap_width), 0, 0])
          cube([length_right, sqrt((gap_width^2) + (height^2)) - 3.343, thickness]);
      left_fill = 4.235;
      translate([0, lip_width + gap_width - left_fill, height - thickness])
        cube([length_right, lip_width + left_fill, thickness]);
   }
    translate([length_right, 0, -1])
      rotate([0, 0, 10])
          cube([50, 100, 50]);
    translate([length_right - 38, 0, 0])
      rotate([0, 0, 10])
        translate([0, -10, -1])
          cube([50, 25, 2 * thickness]);

    for (i = [0 : 2]) {
      translate([lip_width / 2 + (i * (length_right - lip_width - 38) / 2), lip_width / 2, 0])
        cylinder(h = 100, d = 3.5);
      translate([lip_width / 2 + (i * (length_right - lip_width - 14) / 2), lip_width / 2 + gap_width + lip_width, 0])
        cylinder(h = 100, d = 3.5);
    }
  }
}
*/


module ventTopCenter() {
  difference() {
  //  union() {
    cube([192, 71, thickness]);

    translate([97.8, 9.4, -10])
      rotate([0, 0, 10])
        for (i = [0 : 1]) {
          translate([i * 85.5, 0, 0])
            cylinder(h = 100, d = 3.5);
        }

    translate([7.2, 21.6, -10])
      rotate([0, 0, -10])
        for (i = [0 : 1]) {
          translate([i * 70.5, 0, 0])
            cylinder(h = 100, d = 3.5);
        }

    translate([7.2, 65, -10])
      cylinder(h = 100, d = 3.5);

    translate([183, 65, -10])
      cylinder(h = 100, d = 3.5);

    translate([194 / 2 - 8, 0, -0.1])
      for (i = [0, 1])
        mirror([i, 0, 0])
          mirror([0, 1, 0])
            rotate([0, 0, -10])
              cube([110, 30, thickness + 0.2]);

    translate([-5, 1, thickness / 2])  
      cube([20, 71, 5 * thickness]);  

    translate([175, 1, thickness / 2])  
      cube([20, 71, 5 * thickness]);  
  }
}


module ventTopCenter_2() {
  difference() {
  //  union() {
    union() {
      translate([-71.5, 0, 0]) {
        cube([347, 71, thickness]);
        translate([0, 52.6, thickness])
          cube([347, 20, thickness + 1]);
      }  
    }

    translate([175.5, 0, 0]) {
      translate([90, 65, -10])
        cylinder(h = 100, d = 3.5);

      translate([91, 39.1, -10])
        cylinder(h = 100, d = 3.5);
    }

    translate([-153, 0, 0]) {
      translate([90, 65, -10])
        cylinder(h = 100, d = 3.5);

      translate([91, 34.1, -10])
        cylinder(h = 100, d = 3.5);
    }

    translate([87.5, 65, -10])
      for (i = [-1 , 1])
        translate([i * 10, 0, 0])
          cylinder(h = 100, d = 3.5);

    translate([97.8, 9.4, -10])
      rotate([0, 0, 10])
        for (i = [0 : 1]) {
          translate([i * 85.5, 0, 0])
            cylinder(h = 100, d = 3.5);
        }

    translate([7.2, 21.6, -10])
      rotate([0, 0, -10])
        for (i = [0 : 1]) {
          translate([i * 70.5, 0, 0])
            cylinder(h = 100, d = 3.5);
        }

    translate([194 / 2 - 8, 0, -0.1])
      for (i = [0, 1])
        mirror([i, 0, 0])
          mirror([0, 1, 0])
            rotate([0, 0, -10])
              cube([210, 40, thickness + 0.2]);
  }
}

module ventTopLeft() {
  translate([175.2, 0, 0])
   difference() {
 //   union() {
      union() {
        cube([105, 71, thickness]);
        translate([85.4, 48.6, - 2 * thickness]) 
          rotate([0, 0, 10]) 
            cube([14, 20, 2 * thickness]);  
      }       

    translate([-5, 1, - 3 * thickness / 2])  
      cube([20, 71, 2 * thickness]);  

    translate([7.7, 65, -10])
      cylinder(h = 100, d = 3.5);

    translate([90, 65, -10])
      cylinder(h = 100, d = 3.5);

    translate([91, 39.1, -10])
      cylinder(h = 100, d = 3.5);

    translate([6.8, 24.3, -10])
      rotate([0, 0, -10])
        cylinder(h = 100, d = 3.5);


    translate([-5, 14, -0.1])
      mirror([0, 1, 0])
        rotate([0, 0, -10])
          cube([120, 50, thickness + 0.2]);


/*
    translate([194 / 2 - 8, 0, -0.1])
      for (i = [0, 1])
        mirror([i, 0, 0])
          mirror([0, 1, 0])
            rotate([0, 0, -10])
              cube([110, 30, thickness + 0.2]);

    translate([-5, 1, thickness / 2])  
      cube([20, 71, 5 * thickness]);  

    translate([175, 1, thickness / 2])  
      cube([20, 71, 5 * thickness]);  
  */
  }
}

/*
difference() {
  cube([10, 80, 11.5], center = true);
  translate([0, 10, 0])
    cube([3.5, 80, 12], center = true);
}
*/

module ventSectionEndLeft() {
  difference() {
    union() {
      hull() {
        rotate([0, 0, 5.6])
          translate([1.1, lip_width - 0.1, 0])
            cube([70, 0.1, thickness]);
        left_fill = 0.893;
        translate([0, lip_width + gap_width - left_fill, height])
          cube([62, 0.1, thickness]);
      }
//      translate([65, lip_width + 7, 0])
  //      cube([5, 37, 5]);

      rotate([0, 0, 5.6])
        translate([1.1, 0, 0])
          cube([70, lip_width, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([62, lip_width + left_fill, thickness]);
    }
    for (j = [0, 1]) {
      rotate([0, 0, j == 0 ? 5.6 : 0])
        for (i = [0 : 2]) 
          translate([lip_width / 2 + (i * 42) + (j == 0 ? i * 8 : 0), lip_width / 2 + (j * (gap_width + lip_width)), 0])
            cylinder(h = 100, d = 3.5);
    }
    translate([32, lip_width + (gap_width / 2), height / 2 + (thickness / 2)])
      rotate([atan2(height, gap_width), 0, 0])
        airSlotShort();      
  }
}


module ventSectionEndRight() {
  mirror([1, 0, 0])
  difference() {
    union() {
      hull() {
        rotate([0, 0, 4])
          translate([1.1, lip_width - 0.1, 0])
            cube([105, 0.1, thickness]);
        left_fill = 0.893;
        translate([0, lip_width + gap_width - left_fill, height])
          cube([97, 0.1, thickness]);
      }
//      translate([100, lip_width + 7, 0])
//        cube([5, 37, 5]);
      rotate([0, 0, 4])
        translate([1.1, 0, 0])
          cube([105, lip_width, thickness]);
      left_fill = 0.893;
      translate([0, lip_width + gap_width - left_fill, height])
        cube([97, lip_width + left_fill, thickness]);
    }
    for (j = [0, 1]) {
      rotate([0, 0, j == 0 ? 4 : 0])
        for (i = [0 : 2]) 
          translate([lip_width / 2 + (i * (length - lip_width) / 2) + (j == 0 ? i * 8 : 0), lip_width / 2 + (j * (gap_width + lip_width)), 0])
            cylinder(h = 100, d = 3.5);
    }
    translate([length / 4 + 12, lip_width + (gap_width / 2), height / 2 + (thickness / 2)])
      rotate([0, 0, 0]) rotate([atan2(height, gap_width), 0, 0])
        airSlot();      
  }
}


ventSection();
// ventSectionEndLeft();
