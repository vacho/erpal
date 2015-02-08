/**
 *  @version 0.1.0
 */

(function($) {
    
    // Global config
    var pConfig        = {
      images_only         : true  // Paste images only? (Currently not supported)
    };
    
    // Extend config
    /*if( window.ScreenshotPasteConfig )
      $.extend( true, pConfig, window.ScreenshotPasteConfig );*/
    
    // Something like local globals
    var pEditor           = null;     // Any editor instance
    var pDocument         = document; // The document of the parent window
    var pWaitingForInput  = null;     // The currently active/focused contenteditable to take the pasted screenshots from
    
    /**
     *  Check whether the two given buttons are equal
     *  Since we might run with a jQuery version <1.6 we can't rely on $.is()
     *  
     *  @param pButton1 
     *  @param pButton2
     *  @return (bool)
     */
    function buttonsEqual( pButton1, pButton2 ) {
      return pButton1.attr("id")==pButton2.attr("id");
    }
    /**
     *  Set the given buttons text
     *  
     *  @param sText The text to be set
     *  @param pButton The button to set the text to. Defaults to pWaitingForInput
     */
    function setText( sText, pButton, sAppend ) {
      if( !pButton )
        pButton = pWaitingForInput;
      
      if( !pButton )
        return;
      
      pButton
        .removeClass( 'paste-error' )
        .removeClass( 'active' )
        .text('');
      
      if( sText=='unknown_file_type' || sText=='missing_image' || sText=='use_real_browser' )
        pButton
          .addClass( 'paste-error' );
      else if( sText=='press_ctrl_v' )
        pButton
          .addClass( 'active' );
      
      sText = pEditor.lang.screenshot_paste[sText] + (sAppend?sAppend:'');
      
      $('<span class="cke_button_label"></span>')
        .text( sText )
        .appendTo( pButton );
    }
    
    /**
     *  Don't let anyone know about the mousedown event
     *  
     *  @param e The jQuery event object
     *  @return FALSE
     */
    function mousedown(e) {
      e.stopPropagation();
      e.preventDefault();
      return false;
    }
    /**
     *  Don't let anyone know about the mouseup event
     *  
     *  @param e The jQuery event object
     *  @return FALSE
     */
    function mouseup(e) {
      e.stopPropagation();
      e.preventDefault();
      return false;
    }
    /**
     *  Handle contenteditable click event
     *  We simply focus the content
     *  
     *  @param e The jQuery event object
     *  @return FALSE
     */
    function click(e) {
      
      pNewInput = $(e.target);
      if( !pNewInput.hasClass('cke_button__screenshotpaste') )
        pNewInput  = pNewInput.parents( '.cke_button__screenshotpaste' );
      
      if( pWaitingForInput && !buttonsEqual( pWaitingForInput, pNewInput ) )
        setText( 'click_here' );
      
      pWaitingForInput  = pNewInput;
      pWaitingForInput.focus();
      
      e.stopPropagation();
      e.preventDefault();
      return false;
      
    }
    /**
     *  Handle conenteditable blur event
     *  Here we change back the text and unset the global pointing to us
     *  
     *  @param e The jQuery event object
     *  @return FALSE
     */
    function blur(e) {
      setText( 'click_here' );
      
      if( pWaitingForInput && buttonsEqual( pWaitingForInput, $(e.target) ) )
        pWaitingForInput  = null;
    }
    /**
     *  Handle conenteditable focus event
     *  Change text to "PASTE NOW" and select whole text to make sure we don't paste our own translations instead...
     *  
     *  @param e The jQuery event object
     *  @return FALSE
     */
    function focus(e) {
      
      if( pWaitingForInput && !buttonsEqual( pWaitingForInput, $(e.target) ) )
        setText( 'click_here' );
      
      pWaitingForInput  = $(e.target);
      
      setText( 'press_ctrl_v' );
      
      //pWaitingForInput.focus();
      
      if( pWaitingForInput[0].execCommand )
        pWaitingForInput[0].execCommand( 'selectAll', false, null );
      
      //e.stopPropagation();
      //e.preventDefault();
      //return false;
      
    }
    /**
     *  Handle the global paste event
     *  
     *  @param event The jQuery event object
     *  @return FALSE
     */
    function onPaste(event) {
      
      // No input selected... make sure that we don't accidentally display a wrong button text
      if( !pWaitingForInput ) {
        $(pDocument)
          .find('div.cke_button__screenshotpaste')
          .each( function() {
            
            setText( 'click_here', $(this) );
            
          } );
        
        return;
      }
      
      // Chrome: We have access to the files from the clipboard. So why not inserting them?!
      if( event.originalEvent.clipboardData ) {
        var items = event.originalEvent.clipboardData.items;
        
        if( !items )
          items     = event.originalEvent.clipboardData.files;
        
        if( !items.length ) {
          setText( 'missing_image' );
          pWaitingForInput  = false;
          return;
        }
        
        var sType   = items[0].type;
        var bImage  = sType.substr(0,6)=="image/";
        var bText   = sType.substr(0,5)=="text/";
        if( !items[0] || (!bImage && (pConfig.images_only || !bText)) ) {
          if( items[0] ) {
            if( pWaitingForInput )
              setText( 'unknown_file_type', null, items[0].type );
          }
          
          pWaitingForInput  = false;
          
          return;
        }
        
        // Text doesn't need a reader. Just paste it
        if( bText ) {
          items[0].getAsString( function(data) {
            
            if( sType=="text/html" )
              paste( data );
            else
              paste( null, null, data );
            
          } );
          
          return;
        }
        
        // Read blob file, then paste the data url as img-src
        var blob      = items[0].getAsFile();
        var reader    = new FileReader();
        reader.onload = function( event ) {
          
          paste( null, event.target.result );
          
        };
        
        reader.readAsDataURL( blob );
        
        return false;
      }
      else {
        
        // Since firefox hasn't added the screenshot to the contenteditable yet, we wait a milisecond before pasting it
        setTimeout( function() {
          
          var html  = pWaitingForInput.html();
          if( html.indexOf( '<img' )<0 ) {
            setText( 'missing_image' );
            pWaitingForInput  = false;
          }
          else
            paste( html );
          
        }, 1 );
        
        // Firefox would skip pasting it, if we returned false here...
        //return false;
        
      }
      
    }
    
    $( pDocument ).bind( "paste", onPaste );
    
    /**
     *  Paste the given data into the CKEDITOR
     *  
     *  @param html The HTML to paste
     *  @param image The image SRC to be added. Will overwrite any given HTML
     *  @param text The plain text to be added. Will overwrite both html and image
     *  @return NULL
     */
    function paste( html, image, text ) {
      
      var sID = pWaitingForInput.parents( '.cke' ).attr( 'id' ).substr( 4 );
      
      setText( 'click_here' );
      
      if( text ) {
        CKEDITOR.instances[ sID ].insertText( text );
      }
      else {
        if( image )
          html  = "<img src=\""+image+"\" />";
        
        var object      = CKEDITOR.dom.element.createFromHtml( html );
        
        CKEDITOR.instances[ sID ].insertElement( object );
      }
      
      pWaitingForInput  = null;
      
    }
    
    // Global function to transform toolbar buttons into contenteditables
    window._____loadScreenshotPasteButton = function() {
      
      var pButton = $(pDocument).find('a.cke_button__screenshotpaste');
      
      var pEditable = $('<div />');
      
      setText( 'click_here', pEditable );
      
      pButton.each( function() {
        
        var pSelf     = $(this);
        var sID       = pSelf.parents( '.cke' ).attr( 'id' )+'-screenshot-paste';
        var pInstance = pEditable
          .attr( "id", sID )
          .clone()
          .addClass( pSelf.attr('class') )
          .attr( "contenteditable", "" )
          .mousedown( mousedown )
          .click( click )
          .mouseup( mouseup )
          .focus( focus )
          .blur( blur );
        
        pButton
          .replaceWith( pInstance );
        
      } );
      
    };
    
    var pGlobalInited = false;
    
    // Add our plugin
    CKEDITOR.plugins.add( 'screenshot_paste', {
      
      icons : 'click',  // Should never be displayed
      lang  : 'en,de',  // Languages we support
      init  : function( editor ) {
        
        // As taken from http://stackoverflow.com/a/9851769
        var isFirefox = 'MozBoxSizing' in document.documentElement.style;
        var isChrome  = !!(window.chrome && chrome.webstore && chrome.webstore.install);
        // No IE/Opera/Safari etc.
        if( !isFirefox && !isChrome )
          return;
        
        // Config given? Support it.
        /*if( editor.config.screenshot_paste )
          $.extend( true, pConfig, editor.config.screenshot_paste );*/
        
        // For the case we need any editor instance somewhere (like when using translations)
        pEditor = editor;
        
        // Hollow command
        editor.addCommand( 'pasteScreenshot', {
          exec : function( editor )
          {
          }
        } );
        
        /*var add = editor.ui.addButton;
        add.apply( editor.ui, [ 'ScreenshotPaste', {*/
        
        editor.ui.addButton( 'ScreenshotPaste', {
          label: editor.lang.screenshot_paste.click_here,
          command: 'pasteScreenshot',
          icon: this.path + 'click.png'
        } );
        
        // Replace toolbar buttons
        setTimeout( '_____loadScreenshotPasteButton();', 1 );
        
        // Require only once...
        if( !pGlobalInited ) {
          // Add fancy stylesheet
          CKEDITOR.document.appendStyleSheet( CKEDITOR.getUrl( this.path + 'plugin.css' ) );
          
          pGlobalInited = true;
        }
        
      }
      
    } );
    
})(jQuery);

