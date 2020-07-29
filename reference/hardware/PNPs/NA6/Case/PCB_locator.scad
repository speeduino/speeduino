$fn = 100;

// standard board thicknesses (mm)
PCB_THICK_06 = 0.6;
PCB_THICK_08 = 0.8;
PCB_THICK_10 = 1.0;
PCB_THICK_12 = 1.2;
PCB_THICK_16 = 1.6;

// standard board hole diameters
PCB_HOLE_30 = 3.0;

HEIGHT = 1.8;

// standard stand-off heights
STANDOFF_4 = 3.0;

// base dimensions
BASE_THICKNESS = 3.0;
BASE_DIAMETER = 12.0;
  
module pcb_support(coord,hole_dia,pcb_thick,standoff_height)
{
  translate(coord)
  {
    standoff(hole_dia,standoff_height);
    translate([standoff_height,0,0]) clip(hole_dia,pcb_thick,standoff_height);
  }
}

module baseWedge(hole_dia,base_thickness)
{
  standoff_side = hole_dia*1.2;              // width of standoff
  //standoff_thickness = hole_dia * .75;     // thickness of standoff bar
    standoff_thickness = HEIGHT;     // thickness of standoff bar
  
  translate([0,-base_thickness+0.2,0])
    linear_extrude(height=standoff_thickness)
    {
      polygon
      (
        points=
        [
          [-(standoff_side/2+base_thickness),0],
          [-standoff_side/2,base_thickness],
          [standoff_side/2,base_thickness],
          [standoff_side/2+base_thickness,0]
        ]
      );
    }
}

module standoff(hole_dia,height,base_thickness)
{
  standoff_side = hole_dia*1.2;             // width of standoff
  //standoff_thickness = hole_dia * .75;      // thickness of standoff bar
  standoff_thickness = HEIGHT;      // thickness of standoff bar
  shaft_width = hole_dia * 1.0;       // width of clip part that rests inside of the PCB hole
  slot_length = height/5-2;                   // length of cut slot
  slot_width = shaft_width * 0.4;           // amount of gap in the slot
  base_inset = 0.1;                         // inset of wedge into bottom of base
    
  // standoff
  translate([-standoff_side/2,0,0])
  {
    difference()
    {
      cube([standoff_side,height,standoff_thickness]);
      translate([standoff_side/2-slot_width/2,height-slot_length,0]) cube([slot_width,slot_length,standoff_thickness]);
    }
  }
  
  difference()
  {
    baseWedge(hole_dia,base_thickness);
    translate([-(standoff_side+base_thickness*2)/2,-base_thickness,0]) cube([standoff_side+base_thickness*2,base_inset,standoff_thickness]);
  }
}

//pcb_support([0,0,0],3,PCB_THICK_16,8);
module clip(hole_dia,pcb_thick)
{
clip_length = pcb_thick * 3;        // total length of clip
clip_width = hole_dia * 1.2;        // width of clip at its widest
//clip_thickness = hole_dia * .75;    // thickness of of the entire clip in the Z direction
clip_thickness = HEIGHT;    // thickness of of the entire clip in the Z direction
shaft_length = pcb_thick * 1.1;     // length of clip part that rests inside of the PCB hole
shaft_width = hole_dia * 0.9;       // width of clip part that rests inside of the PCB hole
slot_length = clip_length;      // length of cut slot
slot_width = shaft_width * 0.4;    // amount of gap in the slot
  
clip_lip = 0.4;                      // overhange on one side
clip_chamfer = hole_dia/3;           // side of triangle taken off of clip to produce a chamfer  
  
  translate([-shaft_width/2,0,0])
  linear_extrude(height=clip_thickness)
  {
    polygon 
    (
      points=
      [
        // left clip
        [0,0],
        [0,shaft_length],
        [-clip_lip,shaft_length+clip_chamfer],
        [-clip_lip,clip_length-clip_chamfer],
        [0,clip_length],
        [(shaft_width-slot_width)/2,clip_length],
        [(shaft_width-slot_width)/2,clip_length-slot_length],

        // right clip
        [(shaft_width-slot_width)/2+slot_width,clip_length-slot_length],
        [(shaft_width-slot_width)/2+slot_width,clip_length],
        [shaft_width,clip_length],
        [shaft_width+clip_lip,clip_length-clip_chamfer],
        [shaft_width+clip_lip,shaft_length+clip_chamfer],
        [shaft_width,shaft_length],
        [shaft_width,0]
      ]
    );
  }
}

scale([1.8,3.2,2.5])
{
translate([0,STANDOFF_4,0]) clip(PCB_HOLE_30,PCB_THICK_16);
standoff(PCB_HOLE_30,STANDOFF_4,BASE_THICKNESS);
}
