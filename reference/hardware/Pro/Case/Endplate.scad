$fn=50;
height=1;
hole_radius=(3.3782/2)+0.5;
corner_radius=4.00;
//hole_radius=(3.3782/1);

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
        USB_height = 11;
        translate([-corner_radius+29.3,-corner_radius+13,-5])	cube([14,USB_height,10]);
        
        //MAP cutout
        //translate([20,110,screw_window_z+screw_window_height/2-2])	rotate ([90,0,0])   cylinder(r=3,h=screw_window_height);
        translate([-corner_radius+90,-corner_radius+15,-5])	cylinder(r=3,h=10);
        
        //Main loom cutout
        loom_width=84;
        loom_height=20;
        translate([-corner_radius+9,-corner_radius+29.2,-5])    cube([84,20,10]);
        

    } //difference()
}

//projection(cut = true) plate(); //2D projection
plate(); //3D model

