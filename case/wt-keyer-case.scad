/* Which object to generate:
 * 0 = upper part with usb breakout on the pcb
 * 1 = upper part with separate usb breakout
 * 2 = lower part
 * 3 = button prolongation
 * 4 = button label "C"
 * 5 = button label "1"
 * 6 = button label "2"
 */
what = 0;

/* Inner width of the case */
wcase = 70;

/* Inner length of the case */
lcase = 90;

/* Height of the board support: Speaker + components */
hpcbsupport = 20.5 + 11;

/* Inner height of the case.
 * Speaker + components + board + free space.
 */
hcase = hpcbsupport + 1.6 + 2;

/* Height of the lid support */
hlidsupport = hcase;

/* Gap between board and case */
gap = 0.4;

/* Radius for case edges */
rcase = 1.5;

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
xtrimmer1 = 64.5;
ytrimmer1 = 58.0;

/* Trimmer position #2 */
xtrimmer2 = 64.5;
ytrimmer2 = 25.0;

/* Button position #1 */
xbutton1 = 20.5;
ybutton1 = 44;

/* Button position #2 */
xbutton2 = 20.5;
ybutton2 = 35;

/* Button position #3 */
xbutton3 = 20.5;
ybutton3 = 26;

/* Height of button prolongation. Switches are 9mm high, 5mm is overlap. */
hbutton = hpcbsupport + wall - 9.0 + 6.0;

/* Paddle jack position */
xpaddle = 13.0;
ypaddle = hpcbsupport - 3.05;
paddlehole = 6.5;

/* Headphone jack position */
xheadphones = 28.2;
yheadphones = hpcbsupport - 3.05;
headphoneshole = 6.5;

/* USB jack position */
xusb = 41.6;
yusb = hpcbsupport - 0.0;
usbholew = 9.0;
usbholeh = 4.0;

/* Volume control position */
xvolume = 56.0;
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
    translate([-usbholew/2, -usbholeh/2, -eps]) {
        roundedplate(usbholew, usbholeh, wall+2*eps, 1.0);
    }
}

module usb_panel_breakout() {
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

module support() {
    lscrew = 12.0;
    difference() {
        union() {
            translate([0, 0, -eps]) {
                linear_extrude(height = hpcbsupport + eps) {
                    polygon(points=[
                        [-eps, -eps],
                        [-eps, 12 + gap],
                        [6 + gap, 12 + gap],
                        [6 + gap, -eps],
                    ]);
                }
            }
            intersection() {
                translate([-rcase + gap - eps, -rcase -eps, hpcbsupport - eps]) {
                    roundedplate(6 + rcase + eps, 6 + rcase + eps, hlidsupport - hpcbsupport + 0*eps);
                }
                translate([- eps, -eps, hpcbsupport - eps]) {
                    cube([6 + gap + eps, 6 + eps, hlidsupport - hpcbsupport + 2*eps]);
                }
            }
        }
        translate([3.0 + eps, 3.0 + eps, hlidsupport - lscrew]) {
            cylinder_outer(lscrew + eps, mountinghole/2, mountinghole/2);
        }
        translate([3.0 + gap + eps, 9.0 + gap + eps, hpcbsupport - lscrew]) {
            cylinder_outer(lscrew + eps, mountinghole/2, mountinghole/2);
        }
    }
}

module speakerhook() {
    translate([-1.5, 2, 0]) {
        rotate([0, 270, 180])
        linear_extrude(height = 3) {
            /* The polygon might need a height adjustment depending
             * on the speaker and the print resolution.
             */
            polygon(points=[
                [0.0, 0.0], 
                [0.0, 2.0], 
                [1.9, 2.0], 
                [3.3, 4.0], 
                [3.3, 0.0]
            ]);
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
    rotate([0, 0, 275]) {
        translate([0, dspeaker / 2, 0]) {
            speakerhook();
        }
    }
    rotate([0, 0, 145]) {
        translate([0, dspeaker / 2, 0]) {
            speakerhook();
        }
    }
    rotate([0, 0, 35]) {
        translate([0, 9, 0]) {
            translate([0, dspeaker / 2, 0]) {
                speakerscrewholder();
            }
        }
    }
}

module speakerholes() {
    for (angle = [0:11.25:360]) {
        rotate([0, 0, angle]) {
            if (dspeaker/2 > 29+2) {
                translate([29, 0, 0]) {
                    cylinder(wall + 2*eps, r1=1, r2=1);
                }
            }
            if (dspeaker/2 > 25+2) {
                translate([25, 0, 0]) {
                    cylinder(wall + 2*eps, r1=1, r2=1);
                }
            }
            if (dspeaker/2 > 21+2) {
                translate([21, 0, 0]) {
                    cylinder(wall + 2*eps, r1=1, r2=1);
                }
            }
            if (dspeaker/2 > 17+2) {
                translate([17, 0, 0]) {
                    cylinder(wall + 2*eps, r1=1, r2=1);
                }
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
        translate([lcase + wall + gap - 33, wcase/2 + gap + wall, 0]) {
            speakerholes();
        }
        /* Button holes */
        translate([wall + gap + xbutton1 - buttonhole/2, wall + gap + ybutton1 - buttonhole/2, -eps]) {
            cube([buttonhole, buttonhole, wall + 2*eps]);
        }
        translate([wall + gap + xbutton2 - buttonhole/2, wall + gap + ybutton2 - buttonhole/2, -eps]) {
            cube([buttonhole, buttonhole, wall + 2*eps]);
        }
        translate([wall + gap + xbutton3 - buttonhole/2, wall + gap + ybutton3 - buttonhole/2, -eps]) {
            cube([buttonhole, buttonhole, wall + 2*eps]);
        }
        /* Hole for paddle socket */
        translate([-eps, wall + gap + xpaddle, wall + ypaddle]) {
            rotate([90, 90, 90]) {
                cylinder_outer(wall + 2*eps, paddlehole/2, paddlehole/2);
            }
        }
        /* Hole for headphone socket */
        translate([-eps, wall + gap + xheadphones, wall + yheadphones]) {
            rotate([90, 90, 90]) {
                cylinder_outer(wall + 2*eps, headphoneshole/2, headphoneshole/2);
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
        /* Breakout for usb on the pcb */
        if (what == 0) {
            translate([0, wall + gap + xusb, wall + yusb - usbholeh/2]) {
                rotate([90, 0, 90]) {
                    usb_breakout();
                }
            }
        }
        /* Breakout for separate usb socket */
        if (what == 1) {
            translate([0, wall + gap + wcase/2, wall + 10]) {
                rotate([90, 0, 90]) {
                    usb_panel_breakout();
                }
            }
        }
    }
    /* PCB and lid supports */
    translate([wall, wall, wall]) {
        mirror([-1, 1, 0]) {
            support();
        }
    }
    translate([wall + lcase + 2*gap, wall, wall]) {
        rotate([0, 0, 90]) {
            support();
        }
    }
    translate([wall + lcase + 2*gap, wall + wcase + 2*gap, wall]) {
        mirror([1, 1, 0]) {
            support();
        }
    }
    translate([wall, wall + wcase + 2*gap, wall]) {
        rotate([0, 0, 270]) {
            support();
        }
    }
    /* Holders for the speaker */
    translate([lcase + wall + gap - 33, wcase/2 + gap + wall, wall - eps]) {
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
        translate([lcase + 2*gap + groove - 3, 3 + groove, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        translate([3 + groove, wcase + 2*gap + groove - 3, -eps]) {
            cylinder_outer(wall+2*eps, lidhole/2, lidhole/2);
        }
        translate([lcase + 2*gap + groove - 3, wcase + 2*gap + groove - 3, -eps]) {
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

module button() {
    difference() {
        translate([-buttonhole/2, -buttonhole/2, 0]) {
            cube([buttonhole - 0.3, buttonhole - 0.3, hbutton]);
            }
        translate([0, 0, -eps]) {
            cylinder_outer(5.0 + eps, 4.0/2, 3.1/2);
        }
    }
}

module buttonlabel(text) {
    translate([0, 0, hbutton]) {
        label(text, va="center", ha="center");
    }
}

if (what == 0 || what == 1) {
    upper();
}
if (what == 2) {
    lower();
}
if (what == 3) {
    button();
}
if (what == 4) {
    buttonlabel("C");
}
if (what == 5) {
    buttonlabel("1");
}
if (what == 6) {
    buttonlabel("2");
}

