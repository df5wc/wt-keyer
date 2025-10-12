/* Inner width of the case */
wcase = 70;

/* Inner length of the case */
lcase = 90;

/* Height of the board support: Speaker + components */
hpcbsupport = 20.5 + 14;

/* Inner height of the case. 
 * Speaker + components + board + trimmer + safety. 
 */
hcase = hpcbsupport + 1.6 + 7 + 1;

/* Height of the lid support */
hlidsupport = hcase;

/* Gap between board and case */
gap = 0.4;

/* Radius for case edges */
rcase = 1.0;

/* Speaker diameter */
dspeaker = 66;

/* Wall width */
wall = 1.6;

/* Groove width */
groove = 0.6;

/* Screw hole diameter for self cutting screws */
mountinghole = 2.7;

/* Screw hole diameter for lid screws */
lidhole = 3.2;

/* Lower diameter of the feet */
dfoot = 11.0;

/* Height of the feet */
hfoot = 3.0;

/* Trimmer hole diameter */
trimmerhole = 5.0;

/* Button holes */
buttonhole = 6.0;

/* Hidden */
$fn  =  100;
eps  =  0.005;

/* Trimmer position #1 */
xtrimmer1 = 55.7;
ytrimmer1 = 57.8;

/* Trimmer position #2 */
xtrimmer2 = 64.3;
ytrimmer2 = 25.4;

/* Button position #1 */
xbutton1 = 18;
ybutton1 = 44;

/* Button position #2 */
xbutton2 = 18;
ybutton2 = 35;

/* Button position #3 */
xbutton3 = 18;
ybutton3 = 26;

/* Height of button prolongation. Switches are 9mm high, 5mm is overlap. */
hbutton = hpcbsupport + wall - 9.0 + 6.0;

/* Paddle jack position */
xpaddle = 14.6425;
ypaddle = hpcbsupport - 3.05;
paddlehole = 6.5;

/* Volume control position */
xvolume = 54.135;
yvolume = hpcbsupport - 10.0;
volumehole = 7.0;

/* Rig control socket position */
xrigctrl = 35.0;
yrigctrl = hpcbsupport - 10.0;
rigctrlhole = 16.0;


module cylinder_outer(height, r1, r2){
    fudge = 1/cos(180/$fn);
    cylinder(h=height,r1=r1*fudge, r2=r2*fudge);
}

module roundedplate(x, y, z, rad = 0.75) {
    linear_extrude(height = z) {
        hull() {
            translate([0.75, 0.75, 0]) circle(r = 0.75);
            translate([x - 0.75, 0.75, 0]) circle(r = 0.75);
            translate([x - 0.75, y - 0.75, 0]) circle(r = 0.75);
            translate([0.75, y - 0.75, 0]) circle(r = 0.75);
        }
    }
}

module roundedcube(x, y, z, rad = 0.75) {
    hull() {
        translate([    rad,     rad,     rad]) sphere(rad);
        translate([x - rad,     rad,     rad]) sphere(rad);
        translate([    rad, y - rad,     rad]) sphere(rad);
        translate([x - rad, y - rad,     rad]) sphere(rad);
        translate([    rad,     rad, z - rad]) sphere(rad);
        translate([x - rad,     rad, z - rad]) sphere(rad);
        translate([    rad, y - rad, z - rad]) sphere(rad);
        translate([x - rad, y - rad, z - rad]) sphere(rad);
    }
}

module label(text, ha = "center", va = "center") {
    color("orange") {
        translate([0, 0, -eps]) {
            linear_extrude(0.4 + eps) {
                //text(text, size=3, font="Liberation Mono:style=Bold", halign=ha, valign=va);
                //text(text, size=3, font="Arial:style=Bold", halign=ha, valign=va);
                text(text, size=3.0, font="Bitstream Vera Sans Mono:style=Bold", halign=ha, valign=va);
            }
        }
    }
}

module usb_breakout() {
    l = 9;
    h = 4;
    w = 15.8;
    hole = 2.5;
    translate([-l/2, -h/2, -eps]) {
        roundedplate(l, h, wall+2*eps, 1.0);
    }
    translate([-w/2, 0, -eps]) {
        cylinder_outer(wall + 2*eps, hole/2, hole/2);
    }
    translate([w/2, 0, -eps]) {
        cylinder_outer(wall + 2*eps, hole/2, hole/2);
    }
}

module lidsupport(rot) {
    h = hlidsupport + eps;
    lscrew = 12.0;
    rotate([0, 0, rot]) {
        translate([-3 - eps, -3 - eps, 0]) {
            difference() {
                union() {
                    roundedplate(6 + 2*eps, 6 + 2*eps, h);
                    cube([6 + 1.5*eps, 1, h]);
                    cube([1.5, 6 + 2*eps, h]);
                }
                translate([3.0 + eps, 3.0 + eps, h - lscrew]) {
                    cylinder_outer(lscrew + eps, mountinghole/2, mountinghole/2);
                }
            }
        }
    }
}

module pcbsupport(rot) {
    h = hpcbsupport + eps;
    lscrew = 12.0;
    rotate([0, 0, rot]) {
        translate([-3 - eps, -3 - eps, 0]) {
            difference() {
                union() {
                    cube([7 + 2*eps, 7 + 2*eps, h]);
                }
                translate([4.0 + eps, 4.0 + eps, h - lscrew]) {
                    cylinder_outer(lscrew + eps, mountinghole/2, mountinghole/2);
                }
            }
        }
    }
}

module speakerhook() {
    translate([-1.5, 2, 0]) {
        rotate([0, 270, 180]) 
        linear_extrude(height = 3) {
            polygon(points=[[0, 0], [0, 2], [3.0, 2], [4.0, 4], [4.0, 0]]);
        }
    }
}

module speakerscrewholder() {
    translate([-3-eps, -3-eps, -eps]) {
        difference() {
            cube([6 + 2*eps, 6 + 2*eps, 5.5 + eps]);
            translate([3.0 + eps, 3.0 + eps, 0 - eps]) {
                cylinder_outer(5.5 + 3*eps, mountinghole/2, mountinghole/2);
            }
        }
    }
}

module speakerholders() {
    rotate([0, 0, 285]) {
        translate([0, dspeaker / 2 + 1, 0]) {
            speakerhook();
        }
    }
    rotate([0, 0, 155]) {
        translate([0, dspeaker / 2 + 1, 0]) {
            speakerhook();
        }
    }
    rotate([0, 0, 45]) {
        translate([0, 10, 0]) {
            translate([0, dspeaker / 2, 0]) {
                speakerscrewholder();
            }
        }
    }
}

module speakerholes() {
    for (angle = [0:11.25:360]) {
        rotate([0, 0, angle]) {
            translate([29, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
            translate([25, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
            translate([21, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
            translate([17, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
        }
    }
    for (angle = [0:22.5:360]) {
        rotate([0, 0, angle]) {
            translate([13, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
            translate([9, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
        }
    }
    for (angle = [0:45:360]) {
        rotate([0, 0, angle]) {
            translate([5, 0, 0]) {
                cylinder(wall + 2*eps, r1=1, r2=1);
            }
        }
    }
    cylinder(wall + 2*eps, r1=1, r2=1);
}

module upper() {
    x = lcase + 2*gap + 2*wall;
    y = wcase + 2*gap + 2*wall;
    z = hcase + 2*wall + rcase;
    difference() {
        /* Case outer bounds */
        roundedcube(x, y, z, rcase);
        /* Cut off the upper part with the rounded edges */
        translate([-eps, -eps, hcase + 2*wall]) {
            cube([x + 2*eps, y+2*eps, rcase+eps]);
        }
        /* Space inside the case */
        translate([wall, wall, wall]) {
            roundedcube(lcase + 2*gap, wcase+2*gap, hcase+2*wall);
        }
        /* Groove for the lid */
        translate([wall-groove, wall-groove, hcase + wall]) {
            roundedplate(lcase + 2*gap + 2*groove, wcase + 2*gap + 2*groove, wall + eps);
        }
        /* Sound holes for the speaker */
        translate([lcase + wall + gap - dspeaker/2, wcase/2 + gap + wall, 0]) {
            speakerholes();
        }
        /* Button holes */
        translate([wall + gap + xbutton1, wall + gap + ybutton1, -eps]) {
            cylinder_outer(wall + 2*eps, buttonhole/2, buttonhole/2);
        }
        translate([wall + gap + xbutton2, wall + gap + ybutton2, -eps]) {
            cylinder_outer(wall + 2*eps, buttonhole/2, buttonhole/2);
        }
        translate([wall + gap + xbutton3, wall + gap + ybutton3, -eps]) {
            cylinder_outer(wall + 2*eps, buttonhole/2, buttonhole/2);
        }
        /* Hole for paddle socket */
        translate([-eps, wall + gap + xpaddle, wall + ypaddle]) {
            rotate([90, 90, 90]) {
                cylinder_outer(wall + 2*eps, paddlehole/2, paddlehole/2);
            }            
        }   
        /* Hole for volume control */
        translate([-eps, wall + gap + xvolume, wall + yvolume]) {
            rotate([90, 90, 90]) {
                cylinder_outer(wall + 2*eps, volumehole/2, volumehole/2);
            }            
        }   
        /* Hole for rig control socket */
        translate([lcase + wall + 2*gap - eps, wall + gap + xrigctrl, wall + yrigctrl]) {
            rotate([90, 90, 90]) {
                cylinder_outer(wall + 2*eps, rigctrlhole/2, rigctrlhole/2);
            }            
        }   
        /* USB breakout */
        translate([0, wall + gap + wcase/2, wall + 10]) {
            rotate([90, 0, 90]) {
                usb_breakout();
            }            
        }   
    }
    /* Mounting support for the lid */
    translate([wall + 3 - eps, wall + 3 - eps, wall-eps]) {
        lidsupport(0);
    }
    translate([wall + lcase + 2*gap - 3 + eps, wall + 3 - eps, wall-eps]) {
        lidsupport(90);
    }
    translate([wall + lcase + 2*gap - 3 + eps, wall + wcase +2*gap - 3 + eps, wall-eps]) {
        lidsupport(180);
    }
    translate([wall + 3 - eps, wall + wcase +2*gap - 3 + eps, wall-eps]) {
        lidsupport(270);
    }
    /* Mounting support for the board */
    translate([wall + 6 + 3 - eps, wall + 3 - eps, wall-eps]) {
        pcbsupport(0);
    }
    translate([wall + lcase +2*gap - 9 + eps, wall + 3 - eps, wall-eps]) {
        pcbsupport(90);
    }
    translate([wall + lcase +2*gap - 9 + eps, wall + wcase + 2*gap - 3 + eps, wall-eps]) {
        pcbsupport(180);
    }
    translate([wall + 9 - eps, wall + wcase +2*gap - 3 + eps, wall-eps]) {
        pcbsupport(270);
    }
    /* Holders for the speaker */
    translate([lcase + wall + gap - dspeaker/2, wcase/2 + gap + wall, wall - eps]) {
        rotate([0, 0, 90]) {
            speakerholders();
        }
    }
}

module foot() {
    difference() {
        cylinder(hfoot + eps, dfoot/2, (dfoot-1)/2);
        translate([0, 0, hfoot-0.5]) {
            cylinder(1, (dfoot-3)/2, (dfoot-3)/2);
        }
    }
}

module lower() {
    difference() {
        /* Lid */
        roundedplate(lcase + 2*gap + 2*groove, wcase + 2*gap + 2*groove, wall);
        /* Screw holes */
        translate([3 + groove, 3 + groove, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        translate([lcase + gap + groove - 3, 3 + gap + groove, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        translate([3 + groove, wcase + gap + groove - 3, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        translate([lcase + gap + groove - 3, wcase + gap + groove - 3, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        /* Trimmer holes */
        translate([groove + xtrimmer1, groove + ytrimmer1, -eps]) {
            cylinder_outer(wall + 2*eps, trimmerhole/2, trimmerhole/2);
        }   
        translate([groove + xtrimmer2, groove + ytrimmer2, -eps]) {
            cylinder_outer(wall + 2*eps, trimmerhole/2, trimmerhole/2);
        }   
    }
    /* Feet */
    translate([6 + groove + dfoot/2, groove + dfoot/2, wall-eps]) {
        foot();
    }
    translate([lcase - 6 - dfoot/2, groove + dfoot/2, wall-eps]) {
        foot();
    }
    translate([6 + groove + dfoot/2, wcase + groove - dfoot/2, wall-eps]) {
        foot();
    }
    translate([lcase - 6 - dfoot/2, wcase + groove - dfoot/2, wall-eps]) {
        foot();
    }
}

module button(text) {
    union() {
        *difference() {
            cylinder_outer(hbutton, buttonhole/2 - 0.3, buttonhole/2 - 0.3);
            translate([0, 0, -eps]) {
                cylinder_outer(5.0 + eps, 4.0/2, 3.3/2);
            }
        }
        translate([0, 0, hbutton]) {
            label(text, va="center", ha="center");
        }
    }
}

*upper();

*translate([0, 100, 0]) {
    lower();
}

*translate([0, dspeaker / 2, 0]) {
    speakerhook();
}

*usb_breakout();

button("1");
