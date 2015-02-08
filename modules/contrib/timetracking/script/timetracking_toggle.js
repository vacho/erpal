(function ($) {
  
      /**
       * Timer class.
       */
      Timer = function( timetrackingId, button ) {
        this.timerId;
        this.timetrackingId = timetrackingId;
        this.button         = button;
        this.trackingStartTime  = 0;
        this.trackingStartTrack = 0;
        
        // Time fields.
        this.hours    = 0;
        this.minutes  = 0;
        this.seconds  = 0;
        this.setDefaultTime();
      };
      
      /**
       * Timer class methods.
       */
      jQuery.extend( Timer.prototype, {
        
        /**
         *  Start the timer.
         */
        start : function() {
          if( !this.timerId ) {
            this.timerId            = setTimeout( this.updateTime.bind( this ), 1000 );
            this.trackingStartTime  = (new Date()).getTime();
            this.trackingStartTrack = this.getTimeInSeconds();
            
            this.button.attr( 'rel', 'on' );
            this.button.find('.timetracking_button_image').attr( "src", Drupal.settings.timetracking.on.imagepath );
            this.button.find('.timetracking_text').text(Drupal.settings.timetracking.on.linktext);
          }
        },
        
        /**
         *  Get the tracked time in seconds
         */
        getTimeInSeconds  : function() {
          return this.hours * 3600 + this.minutes * 60 + this.seconds;
        },
        
        /**
         *  Check whether or not the timer is currently running
         */
        isRunning : function() {
          return !!this.timerId;
        },
        
        /**
         *  Stop the timer.
         */
        stop  : function() {
          if( this.isRunning() ) {
            clearTimeout( this.timerId );
            delete this.timerId;// Set 'off' image.
            
            this.button.attr( 'rel', 'off' );
            this.button.find('.timetracking_button_image').attr( "src", Drupal.settings.timetracking.off.imagepath );
            this.button.find('.timetracking_text').text(Drupal.settings.timetracking.off.linktext);
          }
        },
        
        /**
         *  Set the time in seconds
         */
        setTimeInSeconds  : function( seconds, skip_update ) {
          if( !skip_update && this.isRunning() ) {
            this.trackingStartTrack += ( seconds - this.getTimeInSeconds() );
          }
          
          this.hours      = Math.floor( seconds / 3600 );
          seconds         -= this.hours * 3600;
          this.minutes    = Math.floor( seconds / 60 );
          seconds         -= this.minutes * 60;
          this.seconds    = Math.floor( seconds );
          
          this.updateDisplay();
        },
        
        /**
         *  Update the time
         */
        updateTime  : function() {
          // Get time passed
          var difference  = (new Date()).getTime() - this.trackingStartTime;
          // Update time components
          var seconds     = Math.floor( difference / 1000 ) + this.trackingStartTrack;
          this.setTimeInSeconds( seconds, true );
          
          // Call again after one second, relative to the timer.
          this.timerId    = setTimeout( this.updateTime.bind( this ), 1000 - (difference%1000) );
        },
        
        // Update timer. Renders current time in h:mm:ss format.
        updateDisplay: function(){
          var time = [];                    
          // Collect time into array.
          if(this.hours > 0) {
            time.push( this.hours );
            time.push( this.formatDigital(this.minutes) );
          }
          else {
            time.push( this.minutes );
          }
          time.push( this.formatDigital(this.seconds) );
          
          this.button.find('.timetracking-time-suffix').text('');
          this.button.find('.timetracking-time').text( time.join( ':' ) );
        },
        
        setDefaultTime: function(){
          var time      = Number( this.button.find('.timetracking-time').attr('data-timetracking-duration') )          
          this.setTimeInSeconds( time * 3600 );
        },
        
        // Digitals in '99' format.
        formatDigital : function( number ){
          return number<10 ? '0'+number : number+'';
        }
      } );
      
      var displaysPerId = {};
  
  
  /**
   *  Due to delays now and then, the timers may actually display different values (only seconds, but anyway).
   *  So we're syncing them after each important event. On load as well since even the server gives different
   *  values for the same timetracking due to high page load times.
   */
  function syncTimers() {
    for( var id in displaysPerId ) {
      if( !displaysPerId.hasOwnProperty(id) )
        continue;
      
      var reference = displaysPerId[id][0];
      for( var i=1; i<displaysPerId[id].length; i++ ) {
        var timer = displaysPerId[id][i];
        timer.setTimeInSeconds( reference.getTimeInSeconds() );
      }
    }
  }
  
  
  /**
   * Timetracking functionality.
   */
  Drupal.behaviors.timetracking = {
    // @TODO Add .detach to remove timers again as well.
    attach: function (context, settings) {
      // Autostart of enabled timer after page load.
      $('.timetracking_button').once('processed')
        .each( function(index, item){
          
          // Get id timer id.
          var id  = $(this).attr('data-timetracking-id');
          
          if( !displaysPerId.hasOwnProperty(id) )
            displaysPerId[id] = [];
          
          timer = new Timer( id, $(this) );
          displaysPerId[id].push( timer );
          
          // Start timer if it was enabled.
          currentState = $(this).attr("rel");
          if(currentState == 'on'){
            timer.start();
          }
        } )
        // Start / Stop on timer button click.
        .click(function () {
          
          // Don't react if timer is in ajax processing.
          if($(this).hasClass('in-progress')) {
            return false;
          }
          
          // Add class for ajax processing.
          $('.timetracking_button').addClass('in-progress');
          
          // Get id of timer.
          var id  = $(this).attr('data-timetracking-id');
          
          // Toggle time tracking.
          timetrackingToggle( id );
          
          return false;
        });
      
      syncTimers();
    }
  };
  
  
  /**
   * Toggle time tracking.
   * 
   * @param {Number}  id  Id of timer.
   */
  function timetrackingToggle( id ) {
    // Just to check if it's running.
    var firstTimer  = displaysPerId[id][0];
    
    $.get( Drupal.settings.timetracking.on.togglepath+id+'/'+(firstTimer.isRunning()?'off':'on')+'/ajax', function(data){	
      
      // Prepare variables for new timer state.
      if( firstTimer.isRunning() )
      {
        for( var i=0; i<displaysPerId[id].length; i++ ) {
          var otherTimer  = displaysPerId[id][i];
          otherTimer.stop();
        }
      }
      else
      {
        for( var otherId in displaysPerId ) {
          if( !displaysPerId.hasOwnProperty(otherId) )
            continue;
          
          for( var i=0; i<displaysPerId[otherId].length; i++ ) {
            var otherTimer  = displaysPerId[otherId][i];
            if( otherId==id ) {
              otherTimer.start();
            }
            else if( otherTimer.isRunning() ) {
              otherTimer.stop();
            }
          }
        }
      }
      
      // Remove class for ajax processing. Processing is finished.
      $('.timetracking_button').removeClass('in-progress');
      
      syncTimers();
    });
  };

})(jQuery);
