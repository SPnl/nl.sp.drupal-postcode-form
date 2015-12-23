<?php

function pcform_admin_form($form, &$form_state) {
  $form['intro'] = array(
    '#type' => 'markup',
    '#markup' => '<h1>Postcode afdelingsinformatiezoeker</h1><p>Zoek afdelingsinformatie bij een lijst met postcodes.</p>',
  );

  $form['postal_codes'] = array(
    '#type' => 'textarea',
    '#title' => 'Postcodelijst',
    '#description' => 'Één (on)geldige postcode per regel. Bijvoorbeeld:<br/>3811jj<br/>3811 jJ<br/>xxxx<br/><br/>7001CP',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

function pcform_admin_form_validate($form, &$form_state) {
  $raw_lines = explode(PHP_EOL, $form_state['values']['postal_codes']);
  foreach ($raw_lines as $key => $line) {
    $line = trim($line);
    if (preg_match('/^([0-9]{4}) *([a-zA-Z]{2})$/', $line, $matches)) {
      $form_state['storage']['postal_codes'][$key] = $matches[1] . ' ' . strtoupper($matches[2]);
    }
    else {
      drupal_set_message('"' . check_plain($line) . '" is geen geldige postcode.', 'warning');
      $form_state['storage']['postal_codes'][$key] = '';
    }
  }
}

function pcform_admin_form_submit($form, &$form_state) {
  foreach ($form_state['storage']['postal_codes'] as $key => $postal_code) {
    if (!empty($postal_code)) {
      $afdeling_info = pcform_afdelingen_search($postal_code);
      if (!empty($afdeling_info)) {
        foreach($afdeling_info as $info) {
          $id = $info->id;
          $data[$key]['postal_code'] = $postal_code;
          $data[$key]['afdeling_info'] = pcform_get_afdeling_data($id);
          break;
        }
      }
      else {
        $data[$key]['postal_code'] = '';
        $data[$key]['afdeling_info'] = array(
          'email' => '',
          'display_name' => '',
        );
      }
    }
    else {
      $data[$key]['postal_code'] = '';
      $data[$key]['afdeling_info'] = array(
        'email' => '',
        'display_name' => '',
      );
    }
  }
  $output = '<table>';
  $output .= '<tr><th></th><th>Postcode</th><th>afdelingsemail</th><th>afdelingsnaam</th></tr>';
  foreach ($data as $key => $values) {
    $output .= "<tr><td>{$key}</td><td>{$values['postal_code']}</td><td>{$values['afdeling_info']['email']}</td><td>{$values['afdeling_info']['display_name']}</td></tr>";
  }
  $output .= '</table>';
  drupal_set_message($output);
}

function pcform_afdelingen_search($string) {
  $string = trim($string);
  if (preg_match('/^([0-9]{4}) ?([a-zA-Z]{2})$/', $string, $matches)) {
    $string = $matches[1] . ' ' . strtoupper($matches[2]);
  }
  $civiapi = spciviapi_get_api();
  $params = array(
    'version' => 3,
    'sequental' => 1,
    'option.limit' => 5,
    'name' => $string,
  );
  $civiapi->Geostelsel->Getafdeling($params);
  $result = $civiapi->result();
  if (isset($result->values)) {
    return $result->values;
  }
}

function pcform_get_afdeling_data($id) {
  $data = array();
  $afdeling = pcform_get_afdeling($id);
  $afdelingsinfo = array(
    'phone',
    'email',
    'street_address',
    'postal_code',
    'city',
  );
  foreach ($afdelingsinfo as $item) {
    if (!empty($afdeling->$item)) {
      $data[$item] = $afdeling->$item;
    }
  }
  if (!empty($afdeling->display_name)) {
    $data['display_name'] = str_replace('SP-afdeling ', '', $afdeling->display_name);
  }
  return $data;
}

function pcform_get_afdeling($id) {
  $civiapi = spciviapi_get_api();
  $params = array(
    'version' => 3,
    'sequental' => 1,
    'id' => $id,
  );
  $civiapi->Contact->Getsingle($params);
  $result = $civiapi->result();
  if (isset($result->display_name)) {
    return $result;
  }
  else {
    return array();
  }
}
