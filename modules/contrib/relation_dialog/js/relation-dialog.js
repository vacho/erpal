(function($){
  
  Drupal.behaviors.relationDialog = {
    attach  : function( context, settings ) {
      $("a.relation-dialog:not(.relation-dialog-processed)")
        .addClass("relation-dialog-processed")
        .click( function(e){
          var pLink = $(e.target);
          
          Drupal.ReferencesDialog.open( pLink.attr("href"), pLink.text() );
          Drupal.ReferencesDialog.entityIdReceived = function(entity_type, entity_id, label) {
            
            var value = pLink.attr('format')
              .replace('$label', label)
              .replace('$entity_id', entity_id)
              .replace('$entity_type', entity_type);
            
            pLink.parents( '.dialog-links' )
              .prev()
              .find( 'input[type="text"]' )
              .val( value );
            
          };
          return false;
          
        } );
    }
  };
  
})(jQuery);