// $fn = 128;
$fn = 64;
//$fn = 32;
//$fn = 16;

thickness = 2;
vent_corner_radius = 5;
vent_width = 141;
vent_height = 46;

dash_corner_radius = 3;
dash_width = 144;
dash_height = 44;

depth = 7;

module housing(width, height, depth, corner_radius) {
  hull()
    for (i = [0,1]) for (j = [0, 1]) mirror([i, 0, 0]) mirror([0, j, 0])
      translate([width / 2 - corner_radius, height / 2 - corner_radius, 0])
        cylinder(h = depth + 0.2, r = corner_radius, center = true);
}

difference() {
  housing(dash_width + (2 * thickness), vent_height + (2 * thickness), depth, dash_corner_radius + thickness);
  housing(dash_width, dash_height, depth + 0.2, dash_corner_radius);
}


translate([0, 0, depth]) difference() {
  housing(dash_width + (2 * thickness), vent_height + (2 * thickness), depth, vent_corner_radius + thickness);
  housing(vent_width, vent_height, depth + 0.2, vent_corner_radius);
}

/*
translate([0, 0, depth]) difference() {
  housing(vent_width + (2 * thickness), vent_height + (2 * thickness), depth, vent_corner_radius + thickness);
  housing(vent_width, vent_height, depth + 0.2, vent_corner_radius);
}
*/
