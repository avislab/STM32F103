<?php
  $fname = "marilyn.png";

  $img=imagecreatefrompng($fname);
  $size = getimagesize ($fname);
  $img_size_x = $size[0];
  $img_size_y = $size[1];
  $img_y8 = ceil($img_size_y / 8);
  print "";
  print "{";
  for ($y8 = 0; $y8 < $img_y8; $y8++) {
    for ($x = 0; $x < $img_size_x; $x++) {
      $hex = 0;
      for ($y = 0; $y < 8; $y++) {
        if ($y8*8+$y < $img_size_y) {
          $rgb = ImageColorAt($img, $x, $y8*8+$y);
        }
        else {          $rgb = 255;
        }
    	  $r = ($rgb >> 16) & 0xFF;
        $g = ($rgb >> 8) & 0xFF;
    	  $b = $rgb & 0xFF;

    	  if (($r+$g+$b) != 0) {    	    $hex = $hex + pow(2,$y);
    	  }
      }
      $hex = '0x'.sprintf("%X", $hex);
      print $hex;
      if ($x < $img_size_x-1) print ",";
    }
    if ($y8 < $img_y8-1) print ",";
  }
  print "}";

?>