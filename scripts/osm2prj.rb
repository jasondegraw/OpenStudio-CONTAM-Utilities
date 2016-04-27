######################################################################
#  Copyright (c) 2008-2016, Alliance for Sustainable Energy.  
#  All rights reserved.
#  
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#  
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
######################################################################
#
# Create a CONTAM (3.1) airflow model from an OpenStudio model
#
# Generate a simplified airflow model from an OpenStudio model. The model that
# is generated is described here:
#   
#   DeGraw, Jason W., Macumber, Daniel, Bahnfleth, William P. Generation of
#   Multizone Airflow Models from Building Energy Models. NY-14-C071. 2014
#   ASHRAE Winter Conference, New York, NY
#
# To summarize:
#
# 1) The airflow model does not contain sketchpad information, so do not
#    expect it to work well with ContamW. Use the model directly with
#    ContamX.
# 2) The airflow model applies leakage to surfaces on a per area basis. This
#    script doesn't do anything with windows or doors, effectively assuming
#    that all windows and doors are closed all the time. Fancier things are
#    possible, but aren't supported here.
# 3) The input OSM should be an "architectural" model where walls are real
#    walls. Perimeter-core zoning is right out.
# 4) When possible, CONTAM's simple air handling systems are generated.
#
# The C++ version of this script is more complex, but was difficult to 
# maintain. Over time, more support for those operations will be available
# in this script and via the Ruby bindings.
#
# Running the script: To run the script, you'll need OpenStudio installed
# and an OpenStudio model. It might be as simple as:
#
#   >ruby osm2prj.rb input.osm
#
# You might need to tell Ruby where to find the OpenStudio libraries, and if
# you don't have Ruby in your path you may need to fix that. You can use the
# OpenStudio Ruby on Windows machines like this:
# 
# >"C:\Program Files\OpenStudio 1.11.1\ruby-install\ruby\bin\ruby.exe" -I "C:\Program Files\OpenStudio 1.11.1\Ruby" osm2prj.rb input.osm
#
# If all goes well, a file named 'output.prj' will be written out containing
# the CONTAM model.
#
require 'openstudio'
usage = 'Usage: ruby osm2prj.rb input.osm'

if ARGV.size != 1
    abort(usage)
end
inpath = ARGV[0]
outpath = "output.prj"

# Try and load the OpenStudio model
vt = OpenStudio::OSVersion::VersionTranslator.new
model = vt.loadModel(OpenStudio::Path.new(inpath))
if model
    model = model.get
else
    abort("Failed to load OSM")
end

# Create the translator
translator = OpenStudio::Airflow::ContamForwardTranslator.new
translator.setAirtightnessLevel("Leaky")
translator.setReturnSupplyRatio(0.9)

# Translate into an index-based model
cx = translator.translateModel(model)
if cx
    cx = cx.get
else
    abort("Translation failed")
end

# Write out the result
outfile = File.open(outpath, "w")
outfile.puts(cx.toString)
outfile.close

