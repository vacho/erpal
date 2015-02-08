#environment = :development
environment = :production

css_dir = "css"
sass_dir = "sass"
images_dir = "images"
fonts_dir = "webfonts"

require 'toolkit'
require 'sass-globbing'

output_style = (environment == :development) ? :expanded : :compressed
relative_assets = true
line_comments = (environment == :production) ? false : true
sass_options = (environment == :development) ? {:debug_info => false} : {:always_update => true}