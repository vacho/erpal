<?php

/**
 * @file
 * Contains a Views field handler to take care of displaying edit links
 * as fields
 */


class timetracking_handler_edit_link_field extends timetracking_handler_link_field {
  function construct() {
    parent::construct();
    $this->additional_fields['type'] = 'type';
  }


  function render($values) {
    $id = $values->timetracking_id;
    $timetracking = timetracking_load($id);

    if (!timetracking_access('edit', $timetracking)) {
      return;
    }
    
    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    $timetracking_id = $values->{$this->aliases['timetracking_id']};
    
    return l($text, 'timetracking/' . $timetracking_id . '/edit', array('query' => array('destination' => $_GET['q'])));
  }
}
