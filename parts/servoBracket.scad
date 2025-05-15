// $fn = 256;
//$fn = 128;
$fn = 64;
//$fn = 32;
// $fn = 16;
//cube([100, 100, 1], center = true);

baseThickness = 6;
wallThickness = 4;
supportLength = 5.5;
servoWidth = 11.8;
servoHeight = 24;
gap = 0.2;
sideMount = true;

module SG90(){
    //colore verde-main color
    color("Blue",0.8){ 
    //corpo principale
    cube([11.8,22.7,22.5]); 
    //ali di fissaggio
    difference(){    
    translate([0,4.3,-4.7]) cube([11.8,2.5,31.9]); 
        translate([5.9,7.3,-2.4])rotate([90,0,0])
        cylinder(r=1,h=3.5,$fn=100);
        translate([5.9,7.3,24.9])rotate([90,0,0])
        cylinder(r=1,h=3.5,$fn=100);
        translate([5.25,4,25.6])cube([1.3,3.2,2.3]);
        translate([5.25,4,-4.7])cube([1.3,3.2,2.3]);
    }
    //cilindro ghiera
    translate([5.9,0,16.6]) rotate([90,0,0])
        cylinder(r=5.9,h=4,$fn=100);
    }
    //cilindro rotante  
    translate([5.9,-4,16.6]) rotate([90,0,0])
        color("White",0.8)cylinder(r=2.3,h=3.2,$fn=100);       
}



/*translate([wallThickness, 11, supportLength + wallThickness])
  mirror([0,1,0])
    SG90();
*/
module servoBracket() {
  translate([0,sideMount ? 23.5 : 0,wallThickness - gap])
    difference() {
      cube([servoWidth + 2 * wallThickness, wallThickness, servoHeight + 2 * supportLength + 5]);
      translate([wallThickness - gap, -gap, supportLength + 5])
        cube([servoWidth + 2 * gap, wallThickness + 2 * gap, servoHeight + 2 * gap]);
      for (i = [0, 1])
        translate([wallThickness + 5.9, -gap, 8 + (i * 29.5)])
          rotate([-90,0,0])
            cylinder(r=1.9,h=4.5);
    }

  difference() {
    translate([servoWidth / 2 + wallThickness, wallThickness / 2, 0]) {
      //cylinder(r = 11,h = 2 * wallThickness);
      
      difference() 
      {
        hull() 
        {
          cylinder(r = 14,h = baseThickness);
          for (i = [0, 1]) mirror([0, i, 0])
            translate([0, 18.75, 0])
              cylinder(r = (sideMount && i == 0 ? 9 : 2.5) + wallThickness,h = baseThickness);
        }
        for (i = [0, 1]) mirror([0, i, 0])
          translate([0, 18.75, -gap])
            cylinder(r = 2.7,h = baseThickness + 2 * gap);
        translate([servoWidth / 2 + wallThickness, -wallThickness / 2, -gap])
          cube([8, wallThickness, baseThickness + 2 * gap]);
      }
    }
  }
}

module servoBracketHorizontal() {
  translate([-10,sideMount ? 23.5 : 0,0])
    difference() {
      cube([servoHeight + 3 * supportLength, wallThickness, servoWidth + 12]);
      translate([supportLength + 2.3, -gap, 6])
        cube([servoHeight + 2 * gap, wallThickness + 2 * gap, servoWidth + 2 * gap]);
      for (i = [0, 1])
        translate([5.3 + (i * 29.5), -gap, 2 * wallThickness + 3.9])
          rotate([-90,0,0])
            cylinder(r=1.9,h=4.5);
    }

  difference() {
    translate([servoWidth / 2 + wallThickness, wallThickness / 2, 0]) {
      //cylinder(r = 11,h = 2 * wallThickness);
      
      difference() 
      {
        hull() 
        {
          cylinder(r = 14,h = baseThickness);
          for (i = [0, 1]) mirror([0, i, 0])
            translate([0, 18.75, 0])
              cylinder(r = (sideMount && i == 0 ? 9 : 2.5) + wallThickness,h = baseThickness);
        }
        for (i = [0, 1]) mirror([0, i, 0])
          translate([0, 18.75, -gap])
            cylinder(r = 2.7,h = baseThickness + 2 * gap);
        translate([servoWidth / 2 + wallThickness, -wallThickness / 2, -gap])
          cube([8, wallThickness, baseThickness + 2 * gap]);
      }
    }
  }
}

module servoBracketDriverSide() {
  translate([11,18.5,wallThickness - gap]) {
    difference() {
      cube([servoWidth + 2 * wallThickness, wallThickness, servoHeight + 2 * supportLength + 5]);
      translate([wallThickness - gap, -gap, supportLength + 5])
        cube([servoWidth + 2 * gap, wallThickness + 2 * gap, servoHeight + 2 * gap]);
      for (i = [0, 1])
        translate([wallThickness + 5.9, -gap, 8 + (i * 29.5)])
          rotate([-90,0,0])
            cylinder(r=1.9,h=4.5);
      translate([-1, 2.5, -gap])
        cylinder(r = 4,h = 7);
    }
  }

  difference() {
    translate([servoWidth / 2 + wallThickness, wallThickness / 2, 0]) {
      //cylinder(r = 11,h = 2 * wallThickness);
      
      difference() 
      {
        hull() 
        {
          cylinder(r = 14,h = baseThickness);
          for (i = [0, 1]) mirror([0, i, 0])
            translate([(i == 0 ? 9 : 0), 18.75, 0])
              cylinder(r = (i == 0 ? 10 : 2.5) + wallThickness,h = baseThickness);
        }
        for (i = [0, 1]) mirror([0, i, 0])
          translate([0, 18.75, -gap])
            cylinder(r = 2.7,h = baseThickness + 2 * gap);
        translate([-10, -wallThickness / 2, -gap])
          cube([20, 2, baseThickness + 2 * gap]);
        translate([-10, 18.75 - 2.7, -gap])
          cube([10, 5.4, baseThickness + 2 * gap]);
      }
    }
  }
}

module driverSideMount()
{
  difference() {
    cylinder(11, 0.8, 3.4);
    translate([0,0,4.5]) difference() {
      cylinder(h = 1.5, d = 5);
      cylinder(h = 1.5, d = 3.44);
    }
    translate([0,0,4.4]) difference() {
      cylinder(h = 0.2, d = 5);
      cylinder(h = 0.2, d = 2);
    }
    translate([0,0,0.5]) difference() {
      cylinder(h = 4, r1 = 0.6, r2 = 1.4);
      cylinder(h = 4, r1 = 0.6, r2 = 1);
    }
    for (i = [0, 1]) for (j = [0, 1]) rotate([0, 0, j * 90]) mirror([0, i, 0])
      translate([-0.15, 1, 2.6])
        cube([0.3, 6, 2]);
    
    translate([0, 0, 7])
      cylinder(h = 5, r = 1.9);
  }
}

module center() {
  difference() {
    union() {
      difference() {
        cube([282, 75, wallThickness], center = true);
        cube([266, 55, wallThickness + 2 * gap], center = true);
      }
      translate([282 / 2 - wallThickness / 2, 0, wallThickness / 2]) {
        rotate([45,0,0])
        cube([wallThickness, 53, 53], center = true);
      }
    }
    translate([0, 0, -wallThickness - 50])
      cube([300, 100, 100 + wallThickness], center = true);
  }
}

module pushPullMount() {
  difference() {
    union() {
      cube([20 + 2 * wallThickness, 3.5 + 2 * wallThickness, 2], center = true);
      translate([-9, 0, 4.4])
        cube([10, 3.5 + 2 * wallThickness, 7], center = true);
    }
    for (i = [0, 1])
      translate([i * 10, 0, -gap])
        cylinder(r = 1.7,h = 3.5 + 2 * wallThickness + 2 * gap, center = true);
    translate([-9, -6, 4])
      rotate([-90,0,0])
        cylinder(r=1.4,h=12);
  }
}

module dashVentBracket() {
  difference() {
    hull() {
      cylinder(r=10,h=wallThickness, center = true);
      translate([-10, 0, 0]) {
        cylinder(r=4,h=wallThickness, center = true);
      }
    }
    translate([-10, 0, 0]) {
      cylinder(r=1.4,h=wallThickness + 0.2, center = true);
    }
    cylinder(r=2.3,h=wallThickness + 0.2, center = true);
    hull() translate([0, 0, wallThickness - 1.5]) {
      cylinder(r=6.5,h=wallThickness + 0.2, center = true);        
      translate([10, 0, 0]) {
        cylinder(r=7,h=wallThickness + 0.2, center = true);
      }
    }
/*
    for (i = [0, 1])
      translate([i * 10, 0, -gap])
        cylinder(r = 1.7,h = 3.5 + 2 * wallThickness + 2 * gap, center = true);
    translate([-9, -6, 4])
      rotate([-90,0,0])
        cylinder(r=1.4,h=12);
*/
  }
}

// center();
// servoBracket();
// servoBracketHorizontal();
// pushPullMount();
servoBracketDriverSide();
// dashVentBracket();
//  driverSideMount();
