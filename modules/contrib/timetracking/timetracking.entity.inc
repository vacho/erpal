<?php


/**
 * Fetch a timetracking object. Make sure that the wildcard you choose 
 * in the timetracking entity definition fits the function name here.
 *
 * @param $timetracking_id
 *   Integer specifying the timetracking id.
 * @param $reset
 *   A boolean indicating that the internal cache should be reset.
 * @return
 *   A fully-loaded $timetracking object or FALSE if it cannot be loaded.
 *
 * @see timetracking_load_multiple()
 */
function timetracking_load($timetracking_id, $reset = FALSE) {
  $timetrackings = timetracking_load_multiple(array($timetracking_id), array(), $reset);
  return reset($timetrackings);
}


/**
 * Load multiple timetrackings based on certain conditions.
 *
 * @param $timetracking_ids
 *   An array of timetracking IDs.
 * @param $conditions
 *   An array of conditions to match against the {timetracking} table.
 * @param $reset
 *   A boolean indicating that the internal cache should be reset.
 * @return
 *   An array of timetracking objects, indexed by timetracking_id.
 *
 * @see entity_load()
 * @see timetracking_load()
 */
function timetracking_load_multiple($timetracking_ids = array(), $conditions = array(), $reset = FALSE) {
  $timetrackings = entity_load('timetracking', $timetracking_ids, $conditions, $reset);
  return $timetrackings;
}


/**
 * Deletes a timetracking.
 */
function timetracking_delete(timetracking $timetracking) {  
  $result = $timetracking->delete();
  return $result;
}

/**
* Returns the duration of the timetracking
*/
function timetracking_get_duration(timetracking $timetracking) {
  return $timetracking->get_duration();
}

/**
 * Delete multiple timetrackings.
 *
 * @param $timetracking_ids
 *   An array of timetracking IDs.
 */
function timetracking_delete_multiple(array $timetracking_ids) {
  entity_get_controller('timetracking')->delete($timetracking_ids);
}


/**
 * Create a timetracking object.
 */
function timetracking_create($values = array()) {
  return entity_get_controller('timetracking')->create($values);
}


/**
 * Saves a timetracking to the database.
 *
 * @param $timetracking
 *   The timetracking object.
 */
function timetracking_save(timetracking $timetracking) {
  return $timetracking->save();
}


/**
 * Saves a timetracking type to the db.
 */
function timetracking_type_save(timetrackingType $type) {
  $type->save();
}


/**
 * Deletes a timetracking type from the db.
 */
function timetracking_type_delete(timetrackingType $type) {
  $type->delete();
}


/**
 * URI callback for timetrackings
 */
function timetracking_uri(timetracking $timetracking){
  return array(
    'path' => 'timetracking/' . $timetracking->timetracking_id,
  );
}


/**
 * Sets up content to show an individual timetracking
 * @todo - get rid of drupal_set_title();
 */
function timetracking_page_view($timetracking, $view_mode = 'full') {    
  $controller = entity_get_controller('timetracking');  
  $content = $controller->view(array($timetracking->timetracking_id => $timetracking));
  return $content;
}


/**
 * The class used for timetracking entities
 */
class timetracking extends Entity {
   
  public function __construct($values = array()) {
    parent::__construct($values, 'timetracking');
  }
  
  public function defaultLabel() {
    if ($duration = $this->get_duration()) {
      $duration_string = timetracking_duration_to_string($duration);
      return t('timetracking !duration h', array('!duration' => $duration_string));
    } else {
      return t('timetracking started');
    }
  }

  protected function defaultUri() {
    return array('path' => 'timetracking/' . $this->timetracking_id);
  }
  
  public function save() {        
    //if not duration is net (not even 0) calculate it and set it.
    if ($this->duration === '' || $this->duration === FALSE || $this->duration === NULL)
      $this->duration = $this->get_duration() / 3600;
    
    parent::save();
  }
  
  /**
  * Timetrackings that are still tracking time have not been stopped and have no endtime so they don't have a duration. For this reason we calculate the duration for not yet stopped timetracking
  */
  public function get_duration() { 
    if (!$this->time_end) {
      $time_end = time();
    }
    else {
      $time_end = $this->time_end;
    }
    
    $duration = $time_end - $this->time_start;
 
    return $duration;
  }
  
  public function delete() {
    
    parent::delete();
  }
  
  /**
  * Closes a timetracking entity set time_end and calculates the duration
  */
  public function close() {
    $time_end = $this->time_end;
    if (!$time_end) {
      $time_end = time();
    }

    //set the duration
    $duration = $this->get_duration($this);
    $this->duration = $duration / (3600);  //need it in hours
    
    $this->time_end = $time_end;
    
    $this->save();
  }
}

/**
* Defining theme functions
*/
function _timetracking_entity_theme() {
  return array();
}

/**
 * The Controller for timetracking entities
 */
class timetrackingController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct($entityType);
  }


  /**
   * Create a timetracking - we first set up the values that are specific
   * to our timetracking schema but then also go through the EntityAPIController
   * function.
   * 
   * @param $type
   *   The machine-readable type of the timetracking.
   *
   * @return
   *   A timetracking object with all default fields initialized.
   */
  public function create(array $values = array()) {
    // Add values that are specific to our timetracking
    $values += array( 
      'timetracking_id' => '',
      'is_new' => true,
      'type' => 'timetracking', //allways fixed because we have no bundles      
      'created' => time(),
      'changed' => time(),
      'time_start' => 0,
      'time_end' => 0,
      'duration' => 0,
      'description' => '',
      'subject_id' => 0,
      'uid' => 0,
    );
    
    $timetracking = parent::create($values);
    return $timetracking;
  }
  
  /**
   * Overriding the buldContent function to add entity specific fields
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $content = parent::buildContent($entity, $view_mode, $langcode, $content);    

    return $content;
  }
  
}
