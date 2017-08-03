include <arduino.scad>

$fn=50;
height=1;
hole_radius=(3.3782/2)+0.5;
corner_radius=4.00;
//hole_radius=(3.3782/1);

//rotate([270,0,0]) translate([17,0,7.5]) arduino(MEGA);

module plate()
{
    difference()
    {
        //Basic outline
        minkowski() {
            cube([95.02,45,height]);
            // rounded corners
            cylinder(r=corner_radius,h=height);
        }

        //Holes
        translate([0,0,-5])    cylinder(r=hole_radius, h=10);
        translate([0,45,-5])    cylinder(r=hole_radius, h=10);
        translate([95.02,0,-5])    cylinder(r=hole_radius, h=10);
        translate([95.02,45,-5])    cylinder(r=hole_radius, h=10);
        
        //USB cutout
        USB_height = 12;
        translate([-corner_radius+28.5,-corner_radius+12.5,-5])	cube([15,USB_height,10]);
        
        //Arduino power cutout
        //translate([-corner_radius+66.3,-corner_radius+18.5,0])	cube([10,13,11],center=true); //right side is x = 45.175, width = 8.9
        
        //MAP cutout
        //translate([20,110,screw_window_z+screw_window_height/2-2])	rotate ([90,0,0])   cylinder(r=3,h=screw_window_height);
        translate([-corner_radius+87,-corner_radius+15,-5])	cylinder(r=5.5,h=10);
        
        //Main loom cutout
        loom_width=84;
        loom_height=20;
        translate([-corner_radius+9,-corner_radius+29.2,-5])    cube([84,20,10]);
        

    } //difference()
}

//projection(cut = true) plate(); //2D projection
plate(); //3D model

