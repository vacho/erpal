<?php

/**
 * @file all functions for configuring the date_item entity repeat features
 */

/**
 * Safe update of date_item src repeat date entity.
 * Looks for changes in rrule and rebuilds date_item's src entity dates
 * Algorithm:
 *   1. clear rrule exclude and include dates;
 *   2. loop dates of src entity;
 *   3. build new rrule with new exclude and include dates.
 * 
 * @param object $entity
 *   date_item entity
 * @param object $src_entity
 *   src of date_item
 * @param array $field
 *   field array of src date where date_item is referenced
 * @param string $field_name
 *   field name of src date where date_item is referenced
 * @param string $l
 *   language code
 * @param bool $remove
 *   remove date_item flag
 * 
 * @return boolean|object
 *   Returns $src_entity if it was updated or FALSE if wasn't
 */
function _date_item_src_update_repeat($entity, $src_entity, $field, $field_name, $l = LANGUAGE_NONE, $remove = FALSE) {
  $rrule = $old_rrule = $src_entity->{$field_name}[$l][0]['rrule'];

  if (!empty($entity->date_items_update) && $entity->date_items_update) {
    return;
  }
  // clear all RDATEs and EXDATEs
  $splitted_rule = explode(chr(13) . chr(10), $rrule);
  $rrule = $splitted_rule[0];

  // parse rrule
  module_load_include('inc', 'date_api', 'date_api_ical');
  module_load_include('inc', 'date_repeat', 'date_repeat_calc');
  $parced_rrule = (array) date_ical_parse_rrule('RRULE:', $rrule);
   
  // get timezone
  $tz = $src_entity->{$field_name}[$l][0]['timezone'];
  $timezone = date_get_timezone($field['settings']['tz_handling'], $tz);

  // get all dateitems of src entity
  $query = db_select('date_item', 'd');
  $query->fields('d', array('date_item_id', 'date_from', 'date_till'));
  $query->condition('src_id', $entity->src_id);
  // check remove date_item flag
  if ($remove) {
    $query->condition('date_item_id', $entity->date_item_id, '!=');
  }
  $date_items = $query->execute()->fetchAll();

  // collect date_items in array
  $datetimes = array();
  foreach ($date_items as $date_item) {
    $date_from = $date_item->date_from;
    $datetimes[] = $date_from;
  }
  // get min / max values
  $min = min($datetimes);
  $max = max($datetimes);
  $count = 0;

  // Add RDATEs in rrule
  foreach ($datetimes as $datetime) {
    if (!date_item_in_rrule($datetime, $parced_rrule)) {
      if ($datetime != $min) {
        _date_item_add_rrule_date($rrule, $datetime, 'RDATE', $timezone, $field);
      }
      else {
        $count++; // add start date that not in rrule
      }
    }
    else {
      $count++; // add date that in rrule
    }
  }
  // update rrule count
  date_item_rrule_set_count($rrule, $count);

  // Add EXDATEs in rrule
  $day = 60 * 60 * 24;
  // find all dates that are between last date and new date item
  for ($time = $min + $day; $time < $max; $time+=$day) {
    // handle only dates that are not in calendar
    if (!in_array($time, $datetimes)) {
      // check only dates whitch can be in rrule
      if (date_item_in_rrule($time, $parced_rrule)) {
        // add each date in excluded dates
        _date_item_add_rrule_date($rrule, $time, 'EXDATE', $timezone, $field);
      }
    }
  }
  // compare updates in rrule
  if ($old_rrule == $rrule) {
    return FALSE;
  }
  // special array that's required by date_repeat_build_dates
  $item = array(
    'value' => $min,
    'value2' => 0, // start and end dates are equal dates in repeat date
    'timezone' => $tz,
  );

  // update src entity dates with new rule
  $dates = date_repeat_build_dates($rrule, NULL, $field, $item);
  $src_entity->{$field_name}[$l] = $dates;

  return $src_entity;
}

/**
 * Check if date_item is in repeat rule days array
 * 
 * @param date $current_day
 *   unix date
 * @param string $rrule
 *   target rrule
 * 
 * @return boolean
 *   TRUE if date in rrule. Otherwice FALSE
 */
function date_item_in_rrule($current_day, $rrule) {
  if (!empty($rrule['BYDAY'])) {
    $BYDAYS = $rrule['BYDAY'];
    foreach ($BYDAYS as $delta => $BYDAY) {
      $BYDAYS[$delta] = substr($BYDAY, -2);
    }
    $current_week_day = date_repeat_dow2day(date('w', $current_day));
    if (in_array($current_week_day, $BYDAYS)) {
      return TRUE;
    }
  }
  if (!empty($rrule['BYYEAR']) && in_array(date_format($current_day, 'Y'), $rrule['BYYEAR'])) {
    return TRUE;
  }
  if (!empty($rrule['BYMONTH']) && in_array(date('n', $current_day), $rrule['BYMONTH'])) {
    return TRUE;
  }
  if (!empty($rrule['BYMONTHDAY'])) {
    // Test month days, but only if there are no negative numbers.
    $test = TRUE;
    $BYMONTHDAYS = array();
    foreach ($rrule['BYMONTHDAY'] as $day) {
      if ($day > 0) {
        $BYMONTHDAYS[] = $day;
      }
      else {
        $test = FALSE;
        break;
      }
    }
    if ($test && !empty($BYMONTHDAYS) && in_array(date('j', $current_day), $BYMONTHDAYS)) {
      return TRUE;
    }
  }
}

/**
 * Adds an exdate to an ical rule
 * TODO: delete it and use _date_item_add_rrule_date
 */
function _date_item_add_exdate($rrule, $unix_time) {
  $splitted_rule = explode(chr(13) . chr(10), $rrule);
  //split each row again
  $exdate_delta = -1;
  foreach ($splitted_rule as $delta => $rule_part) {
    if (strpos($rule_part, 'EXDATE') !== false) {
      $exdate_delta = $delta;
    }
  }

  //if we have an exdate, append the exception, otherwise insert an exdate
  $add_time = date('Ymd', $unix_time) . 'T' . date('His', $unix_time) . 'Z';

  if ($exdate_delta == -1) {
    $splitted_rule[] = 'EXDATE:';
    $exdate_delta = count($splitted_rule) - 1;
    $splitted_rule[$exdate_delta] .= $add_time;
  }
  else {
    //time should only be added if not already added
    if (strpos($splitted_rule[$exdate_delta], $add_time) === false) {
      $splitted_rule[$exdate_delta] .= ',' . $add_time;
    }
  }

  return implode(chr(13) . chr(10), $splitted_rule);
}

/**
 * Adds excluded or included date into repeat rule
 * 
 * @param string $rrule 
 *   repeat rule line
 * @param string $unix_time 
 *   date to delete
 * @param string $type 
 *   target date type to add in:
 *   EXDATE - excluded dates
 *   RDATE - included dates
 * 
 * @return boolean|string
 *   FALSE if item can't be added
 *   Updated $rrule string if item was added
 */
function _date_item_add_rrule_date(&$rrule, $unix_time, $type, $timezone, $field) {
  // Prepare settings
  if ($type == 'EXDATE') // excluded dates
    $part_id = 1;
  elseif ($type == 'RDATE') // included dates
    $part_id = 2;
  else
    return FALSE;

  module_load_include('inc', 'date_api', 'date_api_ical');
  $parts = date_repeat_split_rrule($rrule);

  // Important fix of timezone. We musn't user timezone because it's set to 
  // 'UTC' in date_ical_parse_date
  $timezone = 'UTC';

  // dates separator
  $separator = '';
  // Check if start date is not in rrule dates
  if (!empty($parts[$part_id])) {
    $unix_time_formatted = date('Ymd', $unix_time);
    foreach ($parts[$part_id] as $rrule_date) {
      $rrule_datetime = date_ical_date($rrule_date, $timezone);
      $rrule_date_formatted = date_format($rrule_datetime, 'Ymd');
      // Check if two datetime objects are not set to the same date
      if ($unix_time_formatted == $rrule_date_formatted) {
        return FALSE;
      }
    }
    // Add come if there are more than 1 date
    $separator = ',';
  }

  // Add new include date
  $date = new DateObject($unix_time, $timezone, date_type_format($field['type']));
  date_timezone_set($date, timezone_open($timezone));
  $add_time = date_format($date, DATE_FORMAT_ICAL) . 'Z';
  //$add_time = date(DATE_FORMAT_ICAL, $unix_time) . 'Z';
  // split each row
  $splitted_rule = explode(chr(13) . chr(10), $rrule);
  $rrule_date_delta = -1;

  // find include date delta
  foreach ($splitted_rule as $delta => $rule_part)
    if (strpos($rule_part, $type) !== false) {
      $rrule_date_delta = $delta;
    }

  if ($rrule_date_delta == -1) {
    $splitted_rule[] = $type . ':' . $add_time;
  }
  else {
    $splitted_rule[$rrule_date_delta] .= $separator . $add_time;
  }

  $rrule = implode(chr(13) . chr(10), $splitted_rule);
  return TRUE;
}

/**
 * Set rrule count to sprcific value. Updates $rrule by reference
 * 
 * @param string $rrule
 *   target $rrule
 * @param int $count
 *   count number
 * 
 * @return boolean
 *   Returns FALSE if count was bad
 */
function date_item_rrule_set_count(&$rrule, $count) {
  if ($count < 0 || !is_int($count)) {
    return FALSE;
  }
  $parts = date_repeat_split_rrule($rrule);
  $old_count = $parts[0]['COUNT'];

  $rrule = str_replace('COUNT=' . $old_count, 'COUNT=' . $count, $rrule);
}

/**
 * Deletes excluded or included date from repeat rule
 * 
 * @param string $rrule 
 *   repeat rule line
 * @param string $unix_time
 *   date to delete
 * @param string $type
 *   target date type to remove from:
 *   EXDATE - excluded dates
 *   RDATE - included dates
 * 
 * @return boolean|string
 *   FALSE if item can't be deleted
 *   Updated $rrule string if item was removed
 */
function _date_item_del_rrule_date(&$rrule, $unix_time, $type, $timezone) {
  // Prepare settings
  if ($type == 'EXDATE') {// excluded dates
    $part_id = 1;
  }
  elseif ($type == 'RDATE') {// included dates
    $part_id = 2;
  }
  else {
    return FALSE;
  }

  module_load_include('inc', 'date_api', 'date_api_ical');
  $parts = date_repeat_split_rrule($rrule);
  if (empty($parts[$part_id])) {
    return FALSE;
  }

  // Important fix of timezone. We musn't user timezone because it's set to 
  // 'UTC' in date_ical_parse_date
  $timezone = 'UTC';

  // Check if start date is not in rrule dates
  $remove = FALSE;
  $dates = array();
  $unix_time_formatted = date('Ymd', $unix_time);
  foreach ($parts[$part_id] as $rrule_date) {
    $rrule_datetime = date_ical_date($rrule_date, $timezone);
    $rrule_date_formatted = date_format($rrule_datetime, 'Ymd');
    $rrule_udate = strtotime($rrule_date['datetime']);
    // $rrule_udate = date_format($rrule_datetime, 'U');
    // $rrule_date_formatted = date('Ymd', $rrule_udate);
    // Check if two datetime objects are not set to the same date
    if ($unix_time_formatted == $rrule_date_formatted) {
      $remove = TRUE;
    }
    // save only not mached dates
    else {
      $dates[] = $rrule_udate;
    }
  }
  // exit if there are no matches in dates rules
  if (!$remove) {
    return FALSE;
  }

  // split each rrule row
  $splitted_rule = explode(chr(13) . chr(10), $rrule);
  // find include date delta
  $rrule_date_delta = -1;
  foreach ($splitted_rule as $delta => $rule_part) {
    if (strpos($rule_part, $type) !== false) {
      $rrule_date_delta = $delta;
    }
  }

  // Check if there are include dates just for sure
  if ($rrule_date_delta == -1) {
    return FALSE;
  }

  // if there are no include dates - remove include dates row
  if (empty($dates)) {
    unset($splitted_rule[$rrule_date_delta]);
  }
  // collect and format all included dates
  else {
    $rrule_dates = array();
    foreach ($dates as $date) {
      // format include date
      $ical_date = date(DATE_FORMAT_ICAL, $date) . 'Z';
      $rrule_dates[] = $ical_date;
    }
    $splitted_rule[$rrule_date_delta] = $type . ':' . implode(',', $rrule_dates);
  }

  $rrule = implode(chr(13) . chr(10), $splitted_rule);
  return TRUE;
}
